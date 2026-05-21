/*  semitov-riscv-as, Small RISC-V Assembler.
	Copyright (C) 2025 SemiTO-V Student Group <semitofive@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "instruction.h"

#include "debug.h"
#include "error.h"
#include "hash.h"
#include "symtable.h"
#include "utils.h"
#include "writer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pseudo_expansion {
	const instruction *instructions[2];
	char lines[2][LINE_BUF_LEN];
	size_t count;
} pseudo_expansion;

static void pseudo_expansion_reset(pseudo_expansion *expansion) {
	if (!expansion) {
		return;
	}
	memset(expansion, 0, sizeof(*expansion));
}

static assembler_error pseudo_expansion_add(pseudo_expansion *expansion, const instruction *instr, const char *line) {
	if (!expansion || !instr || !line) {
		return ASSEMBLER_PARSE_ERROR;
	}

	if (expansion->count >= 2) {
		return ASSEMBLER_ERR;
	}

	expansion->instructions[expansion->count] = instr;
	snprintf(expansion->lines[expansion->count], sizeof(expansion->lines[expansion->count]), "%s", line);
	expansion->count++;
	return ASSEMBLER_OK;
}

/**
 * @brief Returns true if the opcode is load/store.
 *
 * @param op Opcode.
 * @return true or false.
 */
static bool is_load_store(const char *op) {
	if (!op) {
		return false;
	}

	return strcmp(op, "lb") == 0 || strcmp(op, "lh") == 0 || strcmp(op, "lw") == 0 || strcmp(op, "lbu") == 0 ||
		   strcmp(op, "lhu") == 0 || strcmp(op, "sb") == 0 || strcmp(op, "sh") == 0 || strcmp(op, "sw") == 0;
}

static assembler_error expand_li_instr(char *rd, int64_t imm64, char *line, size_t len,
									   pseudo_expansion *out_expansion) {
	int32_t imm32 = (int32_t)imm64;
	if (imm32 >= INT12_MIN && imm32 <= INT12_MAX) {
		const instruction *addi = find_instruction("addi", strlen("addi"));
		if (!addi) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, len, "addi %s, zero, %d", rd, imm32);
		return pseudo_expansion_add(out_expansion, addi, line);
	}

	int32_t lo = (imm32 << 20) >> 20;
	int32_t hi_20 = (imm32 - lo) >> 12;

	const instruction *lui = find_instruction("lui", strlen("lui"));
	if (!lui) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "lui %s, %d", rd, hi_20);

	assembler_error err = pseudo_expansion_add(out_expansion, lui, line);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	if (lo != 0) {
		const instruction *addi = find_instruction("addi", strlen("addi"));
		if (!addi) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, len, "addi %s, %s, %d", rd, rd, lo);
		return pseudo_expansion_add(out_expansion, addi, line);
	}

	return ASSEMBLER_OK;
}

static assembler_error expand_la_instr(char *rd, int64_t symbol_addr, int32_t current_pc, char *line, size_t len,
									   pseudo_expansion *out_expansion) {
	// auipc hi_20
	// addi lo_12
	assembler_error err;

	int32_t offset = (int32_t)(symbol_addr - current_pc);
	int32_t lo_12 = (offset << 20) >> 20;
	int32_t hi_20 = (offset - lo_12) >> 12;

	const instruction *auipc = find_instruction("auipc", strlen("auipc"));
	if (!auipc) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "auipc %s, %d", rd, hi_20);

	err = pseudo_expansion_add(out_expansion, auipc, line);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	const instruction *addi = find_instruction("addi", strlen("addi"));
	if (!addi) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "addi %s, %s, %d", rd, rd, lo_12);

	err = pseudo_expansion_add(out_expansion, addi, line);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	return ASSEMBLER_OK;
}

static assembler_error expand_mv_instr(char *rs1, char *rd, char *line, size_t len, pseudo_expansion *out_expansion) {
	// mv is implemented with 'addi rs, r1, 0'
	const instruction *addi = find_instruction("addi", strlen("addi"));
	if (!addi) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "addi %s, %s, 0", rd, rs1);

	return pseudo_expansion_add(out_expansion, addi, line);
}

static assembler_error expand_neg_instr(char *rs1, char *rd, char *line, size_t len, pseudo_expansion *out_expansion) {
	// neg is implemented with 'sub rd, zero, rs1'
	const instruction *sub = find_instruction("sub", strlen("sub"));
	if (!sub) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "sub %s, zero, %s", rd, rs1);

	return pseudo_expansion_add(out_expansion, sub, line);
}

static assembler_error expand_nop_instr(char *line, size_t len, pseudo_expansion *out_expansion) {
	const instruction *addi = find_instruction("addi", strlen("addi"));
	if (!addi) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "addi zero, zero, 0");

	return pseudo_expansion_add(out_expansion, addi, line);
}

static assembler_error expand_not_instr(char *rs1, char *rd, char *line, size_t len, pseudo_expansion *out_expansion) {
	// not is implemented with 'xori rd, rs1, -1'
	const instruction *xori = find_instruction("xori", strlen("xori"));
	if (!xori) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "xori %s, %s, -1", rd, rs1);

	return pseudo_expansion_add(out_expansion, xori, line);
}

static assembler_error expand_seqz_instr(char *rs1, char *rd, char *line, size_t len, pseudo_expansion *out_expansion) {
	// seqz is implemented with 'sltiu rd, rs1, 1'
	const instruction *sltiu = find_instruction("sltiu", strlen("sltiu"));
	if (!sltiu) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "sltiu %s, %s, 1", rd, rs1);

	return pseudo_expansion_add(out_expansion, sltiu, line);
}

static assembler_error expand_snez_instr(char *rs2, char *rd, char *line, size_t len, pseudo_expansion *out_expansion) {
	// snez is implemented with 'sltu rd, x0, rs2'
	const instruction *sltu = find_instruction("sltu", strlen("sltu"));
	if (!sltu) {
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}
	snprintf(line, len, "sltu %s, zero, %s", rd, rs2);

	return pseudo_expansion_add(out_expansion, sltu, line);
}

static assembler_error expand_pseudo_instr(assembler_ctx *ctx, const instruction *instr, const char *lineBuf,
										   uint32_t current_pc, pseudo_expansion *out_expansion) {
	if (!instr || !lineBuf || !out_expansion) {
		return ASSEMBLER_PARSE_ERROR;
	}
	pseudo_expansion_reset(out_expansion);

	char rd[REGISTER_LEN] = {0};
	char rs1[REGISTER_LEN] = {0};
	char rs2[REGISTER_LEN] = {0};
	char line[LINE_BUF_LEN] = {0};
	int64_t imm64 = 0x0;

	if (strcmp(instr->name, "li") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %li", rd, &imm64) != 2) {
			log_msg(LOG_ERROR, "Failed to parse li: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}

		CHECK_IMMEDIATE(imm64, INT32_MIN, INT32_MAX, lineBuf);

		return expand_li_instr(rd, imm64, line, LINE_BUF_LEN, out_expansion);
	}

	if (strcmp(instr->name, "la") == 0) {
		char symbol_name[NAME_LEN] = {0};

		if (sscanf(lineBuf, "%*s %4[^,], %s", rd, symbol_name) != 2) {
			log_msg(LOG_ERROR, "Failed to parse la: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}

		// @TODO: check type, a label can be a variable in the data segment or an address for jump
		// right now it works only with variables
		uint32_t data_vaddr = get_data_vaddr(ctx, ctx->base_vaddr);
		uint32_t symbol_offset;
		if (!symtable_lookup(&ctx->table, symbol_name, &symbol_offset)) {
			return ASSEMBLER_SYM_NOT_FOUND;
		}

		uint32_t symbol_address = data_vaddr + symbol_offset;

		return expand_la_instr(rd, symbol_address, current_pc, line, LINE_BUF_LEN, out_expansion);
	}

	if (strcmp(instr->name, "mv") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse mv: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}

		return expand_mv_instr(rs1, rd, line, LINE_BUF_LEN, out_expansion);
	}

	if (strcmp(instr->name, "neg") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse neg: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}
		return expand_neg_instr(rs1, rd, line, LINE_BUF_LEN, out_expansion);
	}

	if (strcmp(instr->name, "nop") == 0) {
		// nop is implemented with 'addi x0, x0, 0'
		return expand_nop_instr(line, LINE_BUF_LEN, out_expansion);
	}

	if (strcmp(instr->name, "not") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse not: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}

		return expand_not_instr(rs1, rd, line, LINE_BUF_LEN, out_expansion);
	}

	if (strcmp(instr->name, "ret") == 0) {
		// jump is not implemented yet
		log_msg(LOG_ERROR, "RET requires jalr whis is not implemented yet: %s", instr->name);

		return ASSEMBLER_JUMP_NOT_IMPLEMENTED;
	}

	if (strcmp(instr->name, "seqz") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse seqz: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}

		return expand_seqz_instr(rs1, rd, line, LINE_BUF_LEN, out_expansion);
	}

	if (strcmp(instr->name, "snez") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs2) != 2) {
			log_msg(LOG_ERROR, "Failed to parse snez: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}

		return expand_snez_instr(rs2, rd, line, LINE_BUF_LEN, out_expansion);
	}

	log_msg(LOG_ERROR, "Unknown pseudo-instruction: %s", instr->name);

	return ASSEMBLER_UNKNOWN_PSEUDO;
}

static assembler_error parse_register(const char *reg, uint8_t *out_value) {
	uint8_t reg_value = 0x0;
	char trailing = '\0';
	char reg_buf[REGISTER_LEN] = {0};

	if (!reg || !out_value) {
		log_msg(LOG_ERROR, "%s", "Invalid register: (null)");
		return ASSEMBLER_INVALID_REGISTER;
	}

	const char *start = reg;
	while (isspace((unsigned char)*start)) {
		start++;
	}

	size_t len = strcspn(start, " \t\r\n");
	if (len == 0 || len >= sizeof(reg_buf)) {
		log_msg(LOG_ERROR, "Invalid register: %s", reg);
		return ASSEMBLER_INVALID_REGISTER;
	}

	memcpy(reg_buf, start, len);
	reg_buf[len] = '\0';
	reg = reg_buf;

	switch (reg[0]) {
		case 'x':
			if (sscanf(&reg[1], "%hhu%c", &reg_value, &trailing) != 1 || reg_value > 31) {
				log_msg(LOG_ERROR, "Invalid register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			*out_value = reg_value;

			return ASSEMBLER_OK;
		case 'a':
			if (sscanf(&reg[1], "%hhu%c", &reg_value, &trailing) != 1 || reg_value > 7) {
				log_msg(LOG_ERROR, "Invalid register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			// a0-a7 -> x10-x17
			*out_value = reg_value + 10;

			return ASSEMBLER_OK;
		case 's':
			if (reg[1] == 'p') {
				if (strcmp(reg, "sp") != 0) {
					log_msg(LOG_ERROR, "Unknown register: %s", reg);
					return ASSEMBLER_INVALID_REGISTER;
				}
				*out_value = 0x2;

				return ASSEMBLER_OK;
			}
			if (sscanf(&reg[1], "%hhu%c", &reg_value, &trailing) != 1 || reg_value > 11) {
				log_msg(LOG_ERROR, "Invalid register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			// s0/s1 -> x8/x9
			// s2-11 -> x18-27
			*out_value = reg_value < 2 ? reg_value + 8 : reg_value + 16;

			return ASSEMBLER_OK;
		case 't':
			// tp -> x4
			if (reg[1] == 'p') {
				if (strcmp(reg, "tp") != 0) {
					log_msg(LOG_ERROR, "Unknown register: %s", reg);
					return ASSEMBLER_INVALID_REGISTER;
				}
				*out_value = 0x4;

				return ASSEMBLER_OK;
			}
			if (sscanf(&reg[1], "%hhu%c", &reg_value, &trailing) != 1 || reg_value > 6) {
				log_msg(LOG_ERROR, "Invalid register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			// t0-2 -> x5-7
			// t3-6 -> x28-31
			*out_value = reg_value < 3 ? reg_value + 5 : reg_value + 25;

			return ASSEMBLER_OK;
		case 'r':
			if (strcmp(reg, "ra") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			// ra (return address) reg (x1)
			*out_value = 0x1;

			return ASSEMBLER_OK;
		case 'g':
			if (strcmp(reg, "gp") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			// gp -> x3
			*out_value = 0x3;

			return ASSEMBLER_OK;
		case 'z':
			if (strcmp(reg, "zero") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			// zero -> x0
			*out_value = 0x0;

			return ASSEMBLER_OK;
		default: {
			log_msg(LOG_ERROR, "Unknown register: %s", reg);

			return ASSEMBLER_INVALID_REGISTER;
		}
	}
}

static assembler_error handle_r_type_instr(const instruction *instr, char *rs1, uint8_t *rs1_val, char *rs2,
										   uint8_t *rs2_val, char *rd, uint8_t *rd_val, int32_t *res) {
	assembler_error err = parse_register(rs2, rs2_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	err = parse_register(rs1, rs1_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	err = parse_register(rd, rd_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	*res = ASSEMBLE_R_TYPE(instr, *rs2_val, *rs1_val, *rd_val);

	return err;
}

static assembler_error handle_i_type_instr(const instruction *instr, char *rs1, uint8_t *rs1_val, int32_t imm, char *rd,
										   uint8_t *rd_val, int32_t *res) {
	assembler_error err = parse_register(rs1, rs1_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	err = parse_register(rd, rd_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	*res = ASSEMBLE_I_TYPE(instr, *rs1_val, imm, *rd_val);

	return err;
}

static assembler_error handle_s_type_instr(const instruction *instr, char *rs1, uint8_t *rs1_val, char *rs2,
										   uint8_t *rs2_val, int32_t imm, int32_t *res) {
	assembler_error err = parse_register(rs2, rs2_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	err = parse_register(rs1, rs1_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	uint32_t imm_u = (uint32_t)imm;

	*res = ASSEMBLE_S_TYPE(instr, *rs1_val, *rs2_val, imm_u);

	return err;
}

static assembler_error handle_b_type_instr(const instruction *instr, char *rs1, uint8_t *rs1_val, char *rs2,
										   uint8_t *rs2_val, int32_t imm, int32_t *res) {
	assembler_error err = parse_register(rs1, rs1_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	err = parse_register(rs2, rs2_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}
	uint32_t imm_u = (uint32_t)imm;

	*res = ASSEMBLE_B_TYPE(instr, *rs1_val, *rs2_val, imm_u);

	return err;
}

static assembler_error handle_u_type_instr(const instruction *instr, char *rd, uint8_t *rd_val, int32_t imm,
										   int32_t *res) {
	assembler_error err = parse_register(rd, rd_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}

	*res = ASSEMBLE_U_TYPE(instr, *rd_val, imm);

	return err;
}

static assembler_error handle_j_type_instr(const instruction *instr, char *rd, uint8_t *rd_val, int32_t imm,
										   int32_t *res) {
	assembler_error err = parse_register(rd, rd_val);
	if (err != ASSEMBLER_OK) {
		return err;
	}
	uint32_t imm_u = (uint32_t)imm;

	*res = ASSEMBLE_J_TYPE(instr, *rd_val, imm_u);

	return err;
}

static assembler_error encode(const instruction *instr, const char *lineBuf, const char *name, int32_t *out_encoded) {
	if (!instr || !lineBuf || !name || !out_encoded) {
		return ASSEMBLER_PARSE_ERROR;
	}

	char rd[REGISTER_LEN] = {0};
	char rs1[REGISTER_LEN] = {0};
	char rs2[REGISTER_LEN] = {0};
	long imm_long = 0;
	int32_t res = 0;
	uint8_t rd_val = 0;
	uint8_t rs1_val = 0;
	uint8_t rs2_val = 0;
	assembler_error err = ASSEMBLER_OK;
	int scan_res = 0;

	switch (instr->type) {
		case R_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %4[^,], %4[^#\n]", rd, rs1, rs2) != 3) {
				log_msg(LOG_ERROR, "Failed to parse R-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}
			log_msg(LOG_DEBUG, "R-Type Read: %s %s %s", rd, rs1, rs2);

			err = handle_r_type_instr(instr, rs1, &rs1_val, rs2, &rs2_val, rd, &rd_val, &res);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			break;
		case I_TYPE:
			// Load opcodes have a specific syntax opcode, reg, offset(reg)
			if (is_load_store(name)) {
				scan_res = sscanf(lineBuf, "%*s %4[^,], %li(%4[^)])", rd, &imm_long, rs1);
				// Probably an implicit zero offset immediate
				if (scan_res < 3) {
					scan_res = sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1);
					if (scan_res != 2) {
						log_msg(LOG_ERROR, "Failed to parse load: %s", lineBuf);
						return ASSEMBLER_PARSE_ERROR;
					}
					imm_long = 0;
				}
			} else {
				if (sscanf(lineBuf, "%*s %4[^,], %4[^,], %li", rd, rs1, &imm_long) != 3) {
					log_msg(LOG_ERROR, "Failed to parse I-type: %s", lineBuf);
					return ASSEMBLER_PARSE_ERROR;
				}
			}

			CHECK_IMMEDIATE(imm_long, INT12_MIN, INT12_MAX, lineBuf);

			log_msg(LOG_DEBUG, "I-Type Read: %s, %s, %li", rd, rs1, imm_long);

			err = handle_i_type_instr(instr, rs1, &rs1_val, (int32_t)imm_long, rd, &rd_val, &res);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			break;
		case S_TYPE:
			// Store opcodes have a specific syntax opcode, src, offset(dest)
			scan_res = sscanf(lineBuf, "%*s %4[^,], %li(%4[^)])", rs2, &imm_long, rs1);
			// Assuming an implicit zero offset immediate
			if (scan_res < 3) {
				scan_res = sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rs2, rs1);
				if (scan_res != 2) {
					log_msg(LOG_ERROR, "Failed to parse S-type: %s", lineBuf);
					return ASSEMBLER_PARSE_ERROR;
				}
				imm_long = 0;
			}

			CHECK_IMMEDIATE(imm_long, INT12_MIN, INT12_MAX, lineBuf);

			log_msg(LOG_DEBUG, "S-Type Read: %li, %s, %s", imm_long, rs1, rs2);

			err = handle_s_type_instr(instr, rs1, &rs1_val, rs2, &rs2_val, (int32_t)imm_long, &res);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			break;
		case B_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %4[^,], %li", rs1, rs2, &imm_long) != 3) {
				log_msg(LOG_ERROR, "Failed to parse B-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}

			CHECK_IMM_AND_LAST_B(imm_long, INT13_MIN, INT13_MAX, lineBuf, "branch");

			log_msg(LOG_DEBUG, "B-Type Read: %s, %s, %li", rs1, rs2, imm_long);

			err = handle_b_type_instr(instr, rs1, &rs1_val, rs2, &rs2_val, (int32_t)imm_long, &res);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			break;
		case U_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %li", rd, &imm_long) != 2) {
				log_msg(LOG_ERROR, "Failed to parse U-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}

			CHECK_IMMEDIATE(imm_long, INT20_MIN, INT20_MAX, lineBuf);

			log_msg(LOG_DEBUG, "U-Type Read: %s, %li", rd, imm_long);

			err = handle_u_type_instr(instr, rd, &rd_val, (int32_t)imm_long, &res);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			break;
		case J_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %li", rd, &imm_long) != 2) {
				log_msg(LOG_ERROR, "Failed to parse J-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}

			CHECK_IMM_AND_LAST_B(imm_long, INT21_MIN, INT21_MAX, lineBuf, "jump");

			log_msg(LOG_DEBUG, "J-Type Read: %s, %li", rd, imm_long);

			err = handle_j_type_instr(instr, rd, &rd_val, (int32_t)imm_long, &res);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			break;
		case Z_TYPE:
			/*
			 * This is a special case for ecall/ebreak, in the reference card
			 * they are in the I type but without operands, so we made a new type (Z)
			 */
			res = ASSEMBLE_Z_TYPE(instr);
			break;
		default:
			log_msg(LOG_ERROR, "Unknown instruction type: %c", instr->type);

			return ASSEMBLER_ERR;
	}

	log_msg(LOG_DEBUG, "Write: %08x", res);
	*out_encoded = res;

	return ASSEMBLER_OK;
}

static assembler_error encode_pseudo_instr(pseudo_expansion expansion, int32_t *encoded, size_t *counter) {
	assembler_error err = ASSEMBLER_OK;
	for (size_t i = 0; i < expansion.count; i++) {
		err = encode(expansion.instructions[i], expansion.lines[i], expansion.instructions[i]->name, &encoded[i]);
		if (err != ASSEMBLER_OK) {
			break;
		}

		log_msg(LOG_DEBUG, "%02lx:\t%08x\t%s", (unsigned long)*counter, encoded[i], expansion.lines[i]);
		*counter += 4;
	}
	return err;
}

static assembler_error handle_data_integer(uint8_t *data, char *directive, char *args, size_t *current_size,
										   size_t capacity) {
	size_t step = 0;
	if (strcmp(directive, ".byte") == 0) {
		step = 1;
	} else if (strcmp(directive, ".half") == 0) {
		step = 2;
	} else {
		step = 4;
	}

	char *token = strtok(args, ",");
	while (token) {
		long long val = 0;
		if (sscanf(token, " %lli", &val) != 1) {
			log_msg(LOG_ERROR, "Invalid numeric literal: %s", token);
			return ASSEMBLER_PARSE_ERROR;
		}

		if (*current_size + step > capacity) {
			log_msg(LOG_ERROR, "Data segment capacity exceeded (%zu bytes max)", capacity);
			return ASSEMBLER_ERR;
		}

		// Write into bytes array
		memcpy(&data[*current_size], &val, step);
		*current_size += step;
		token = strtok(NULL, ",");
	}

	return ASSEMBLER_OK;
}

static assembler_error handle_data_string(uint8_t *data, char *args, char *line, size_t *current_size,
										  size_t capacity) {
	char *start_quote = strchr(args, '"');
	char *end_quote = NULL;
	if (start_quote) {
		end_quote = strrchr(start_quote + 1, '"');
	}

	if (!start_quote || !end_quote) {
		log_msg(LOG_ERROR, "Missing or malformed string literals: %s", line);
		return ASSEMBLER_PARSE_ERROR;
	}

	// Parse inside quotes
	for (char *p = start_quote + 1; p < end_quote; p++) {
		if (*current_size >= capacity) {
			log_msg(LOG_ERROR, "%s", "Data segment capacity exceeded during string parsing");
			return ASSEMBLER_ERR;
		}

		if (*p == '\\' && (p + 1) < end_quote) {
			p++;
			switch (*p) {
				case 'n':
					data[(*current_size)++] = '\n';
					break;
				case 't':
					data[(*current_size)++] = '\t';
					break;
				case 'r':
					data[(*current_size)++] = '\r';
					break;
				case '\\':
					data[(*current_size)++] = '\\';
					break;
				case '\"':
					data[(*current_size)++] = '\"';
					break;
				default:
					data[(*current_size)++] = '\\';
					data[(*current_size)++] = *p;
					break;
			}
		} else {
			data[(*current_size)++] = *p;
		}
	}

	if (*current_size >= capacity) {
		log_msg(LOG_ERROR, "%s", "Data segment capacity exceeded for string null-terminator");
		return ASSEMBLER_ERR;
	}
	data[(*current_size)++] = '\0';

	return ASSEMBLER_OK;
}

static assembler_error handle_data_space(uint8_t *data, char *args, size_t *current_size, size_t capacity) {
	char *count_token = strtok(args, ",");
	if (!count_token) {
		log_msg(LOG_ERROR, "%s", "Missing size for .space directive");
		return ASSEMBLER_PARSE_ERROR;
	}

	long long count = 0;
	if (sscanf(count_token, " %lli", &count) != 1 || count < 0) {
		log_msg(LOG_ERROR, "Invalid size for .space directive: %s", count_token);
		return ASSEMBLER_PARSE_ERROR;
	}

	long long fill = 0;
	char *fill_token = strtok(NULL, ",");
	if (fill_token) {
		if (sscanf(fill_token, " %lli", &fill) != 1 || fill < 0 || fill > UINT8_MAX) {
			log_msg(LOG_ERROR, "Invalid fill byte for .space directive: %s", fill_token);
			return ASSEMBLER_PARSE_ERROR;
		}
	}

	if ((size_t)count > capacity - *current_size) {
		log_msg(LOG_ERROR, "Data segment capacity exceeded (%zu bytes max)", capacity);
		return ASSEMBLER_ERR;
	}

	memset(&data[*current_size], fill, count);
	*current_size += count;

	return ASSEMBLER_OK;
}

static assembler_error parse_data_directive(uint8_t *data, const char *directive, char *args, char *line,
											size_t *current_size, size_t capacity) {
	// Handle data types
	if (strcmp(directive, ".byte") == 0 || strcmp(directive, ".half") == 0 || strcmp(directive, ".word") == 0) {
		return handle_data_integer(data, (char *)directive, args, current_size, capacity);
	} else if (strcmp(directive, ".string") == 0 || strcmp(directive, ".asciz") == 0 ||
			   strcmp(directive, ".asciiz") == 0) {
		return handle_data_string(data, args, line, current_size, capacity);
	} else if (strcmp(directive, ".space") == 0) {
		return handle_data_space(data, args, current_size, capacity);
	} else {
		log_msg(LOG_ERROR, "Unknown or unsupported data directive: %s", directive);
		return ASSEMBLER_UNKNOWN_PSEUDO;
	}
}

static assembler_error process_segment_data(assembler_ctx *ctx, char *line, size_t *data_offset) {
	if (!ctx || !line) {
		return ASSEMBLER_NULL_ERROR;
	}

	if (!data_offset) {
		return ASSEMBLER_NULL_ERROR;
	}

	char *p = line;
	while (*p && isspace((unsigned char)*p)) {
		p++;
	}

	// Get the label if it exists (256 chars)
	char first_token[LINE_BUF_LEN] = {0};
	if (sscanf(p, "%255s", first_token) == 1) {
		size_t len = strlen(first_token);
		if (len > 0 && first_token[len - 1] == ':') {
			// Label found
			p += len;
			while (*p && isspace((unsigned char)*p)) {
				p++;
			}
		}
	}

	// Parse the directive
	char directive[NAME_LEN] = {0};
	if (sscanf(p, "%63s", directive) != 1) {
		log_msg(LOG_ERROR, "Failed to parse data directive: %s", p);
		return ASSEMBLER_PARSE_ERROR;
	}

	// Get directive's arguments
	char *args = p + strcspn(p, " \t");
	while (*args && isspace((unsigned char)*args)) {
		args++;
	}

	size_t current_size = *data_offset;
	size_t capacity = ctx->segments[SEGMENT_DATA].capacity;
	uint8_t *data = ctx->segments[SEGMENT_DATA].data;

	assembler_error err = parse_data_directive(data, directive, args, line, &current_size, capacity);

	if (err == ASSEMBLER_OK) {
		*data_offset = current_size;
	}

	return err;
}

static assembler_error process_segment_text(assembler_ctx *ctx, char *line, size_t *text_offset) {
	assembler_error err = ASSEMBLER_OK;
	char name[NAME_LEN] = {0};
	const instruction *instr = NULL;

	if (!ctx || !line || !text_offset) {
		return ASSEMBLER_NULL_ERROR;
	}

	size_t counter = *text_offset;
	int32_t encoded[2] = {0};

	if (sscanf(line, " %63s ", name) != 1) {
		log_msg(LOG_ERROR, "Failed to parse instruction name: %s", line);
		return ASSEMBLER_PARSE_ERROR;
	}

	instr = find_instruction(name, strlen(name));
	if (!instr) {
		log_msg(LOG_ERROR, "Unknown instruction: %s", name);
		return ASSEMBLER_UNKNOWN_INSTRUCTION;
	}

	size_t instructions_to_copy = 0;

	if (instr->type == 'P') {
		log_msg(LOG_INFO, "expanding pseudo-instruction: %s", name);
		pseudo_expansion expansion;

		uint32_t text_vaddr = get_text_vaddr(ctx->base_vaddr);
		uint32_t current_pc = text_vaddr + *text_offset;

		err = expand_pseudo_instr(ctx, instr, line, current_pc, &expansion);
		if (err != ASSEMBLER_OK) {
			return err;
		}

		err = encode_pseudo_instr(expansion, encoded, &counter);
		if (err != ASSEMBLER_OK) {
			return err;
		}

		instructions_to_copy = expansion.count;
	} else {
		log_msg(LOG_INFO, "fetching instruction: %s (%c TYPE)", name, instr->type);

		err = encode(instr, line, name, &encoded[0]);
		if (err != ASSEMBLER_OK) {
			return err;
		}

		log_msg(LOG_DEBUG, "%02lx:\t%08x\t%s", (unsigned long)counter, encoded[0], line);
		counter += 4;
		instructions_to_copy = 1;
	}

	// Copy bytes into text bytes array and update size
	uint8_t *ep = (uint8_t *)encoded;
	size_t total_bytes = instructions_to_copy * 4;
	size_t current_size = *text_offset;

	for (size_t i = 0; i < total_bytes; ++i) {
		if (current_size >= ctx->segments[SEGMENT_TEXT].capacity) {
			log_msg(LOG_ERROR, "Text segment capacity exceeded (%zu bytes max)", ctx->segments[SEGMENT_TEXT].capacity);
			return ASSEMBLER_ERR;
		}

		ctx->segments[SEGMENT_TEXT].data[current_size++] = ep[i];
	}
	*text_offset = current_size;

	return err;
}

static assembler_error process_segment_bss(assembler_ctx *ctx, char *line) {
	(void)ctx;
	(void)line;

	// @TODO: write function body

	return ASSEMBLER_OK;
}

static assembler_error process_assembly_line(assembler_ctx *ctx, char *segment_name, char *line, size_t *text_offset,
											 size_t *data_offset) {
	if (!segment_name || segment_name[0] == '\0' || strcmp(segment_name, "text") == 0) {
		// Process text segment if the segment is not provided
		return process_segment_text(ctx, line, text_offset);
	}

	if (strcmp(segment_name, "data") == 0) {
		return process_segment_data(ctx, line, data_offset);
	}

	if (strcmp(segment_name, "bss") == 0) {
		return process_segment_bss(ctx, line);
	}

	log_msg(LOG_ERROR, "Unknown segment name: %s", segment_name);
	return ASSEMBLER_ERR;
}

assembler_error assemble_file(const char *filename, assembler_ctx *ctx) {
	if (!filename || !ctx) {
		return ASSEMBLER_NULL_ERROR;
	}

	assembler_error err = ASSEMBLER_OK;
	char line_buffer[LINE_BUF_LEN] = {0};
	char segment[LINE_BUF_LEN] = {0};
	size_t text_offset = 0;
	size_t data_offset = 0;

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		log_msg(LOG_ERROR, "Failed to open file: %s", filename);
		return ASSEMBLER_FILE_ERROR;
	}

	while (fgets(line_buffer, sizeof(line_buffer), fp)) {
		char *line = string_trim(line_buffer);
		if (is_ignorable_line(line)) {
			continue;
		}

		if (line[0] == '.') {
			// If line starts with a dot then is a segment
			// assign it to segment and go to the next line
			snprintf(segment, sizeof(segment), "%s", line + 1);
			continue;
		}

		err = process_assembly_line(ctx, segment, line, &text_offset, &data_offset);
		if (err != ASSEMBLER_OK) {
			break;
		}
	}

	ctx->segments[SEGMENT_TEXT].size = text_offset;
	ctx->segments[SEGMENT_DATA].size = data_offset;

	fclose(fp);

	return err;
}

assembler_error scan_labels(const char *filename, assembler_ctx *ctx) {
	if (!filename || !ctx) {
		return ASSEMBLER_NULL_ERROR;
	}

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return ASSEMBLER_FILE_ERROR;
	}

	// Starts from zero, we don't know how big the segments will be
	uint32_t data_offset = 0;
	uint32_t text_offset = 0;

	char line[LINE_BUF_LEN];
	char label[LABEL_LEN];

	char segment[LINE_BUF_LEN] = {0};

	while (fgets(line, sizeof(line), fp)) {
		char *trimmed = string_trim(line);
		if (is_ignorable_line(trimmed)) {
			continue;
		}

		if (trimmed[0] == '.') {
			snprintf(segment, sizeof(segment), "%s", trimmed + 1);
			continue;
		}

		char *p = trimmed;
		while (*p && isspace((unsigned char)*p)) {
			p++;
		}

		char first_token[LINE_BUF_LEN] = {0};
		if (sscanf(p, "%255s", first_token) == 1) {
			size_t len = strlen(first_token);
			if (len > 0 && first_token[len - 1] == ':') {
				// Extract label name
				size_t copy_len = len - 1;
				if (copy_len >= LABEL_LEN) {
					copy_len = LABEL_LEN - 1;
				}
				memset(label, 0, sizeof(label));
				memcpy(label, first_token, copy_len);

				uint32_t addr = 0;
				if (segment[0] == '\0' || strcmp(segment, "text") == 0) {
					addr = text_offset;
				} else if (strcmp(segment, "data") == 0) {
					addr = data_offset;
				} else {
					log_msg(LOG_ERROR, "Unknown segment name while scanning labels: %s", segment);
					fclose(fp);
					return ASSEMBLER_ERR;
				}

				if (!symtable_insert(&ctx->table, label, addr)) {
					log_msg(LOG_ERROR, "Failed to insert symbol: %s", label);
					fclose(fp);
					return ASSEMBLER_ERR;
				}

				ctx->table.symbols[ctx->table.count - 1].type =
					(segment[0] == '\0' || strcmp(segment, "text") == 0) ? SYM_SEGMENT_TEXT : SYM_SEGMENT_DATA;

				// Advance
				p += len;
				while (*p && isspace((unsigned char)*p)) {
					p++;
				}

				// End of the line
				if (*p == '\0') {
					continue;
				}
			}
		}

		// Parse content
		if (segment[0] == '\0' || strcmp(segment, "text") == 0) {
			char name[NAME_LEN] = {0};
			if (sscanf(p, " %63s", name) != 1) {
				continue;
			}

			const instruction *instr = find_instruction(name, strlen(name));
			if (!instr) {
				log_msg(LOG_ERROR, "Unknown instruction in scan_labels: %s", name);
				fclose(fp);
				return ASSEMBLER_UNKNOWN_INSTRUCTION;
			}

			if (instr->type == 'P') {
				pseudo_expansion expansion;
				uint32_t text_vaddr = get_text_vaddr(ctx->base_vaddr);
				uint32_t current_pc = text_vaddr + text_offset;

				assembler_error err = expand_pseudo_instr(ctx, instr, p, current_pc, &expansion);
				if (err != ASSEMBLER_OK) {
					fclose(fp);
					return err;
				}
				text_offset += (uint32_t)(expansion.count * 4);
			} else {
				text_offset += 4;
			}
		} else if (strcmp(segment, "data") == 0 || strcmp(segment, "bss") == 0) {
			char directive[NAME_LEN] = {0};
			if (sscanf(p, " %63s", directive) != 1) {
				log_msg(LOG_ERROR, "Failed to parse data directive: %s", p);
				fclose(fp);
				return ASSEMBLER_PARSE_ERROR;
			}

			char *args_ptr = p + strcspn(p, " \t");
			while (*args_ptr && isspace((unsigned char)*args_ptr)) {
				args_ptr++;
			}
			char args_copy[LINE_BUF_LEN] = {0};
			snprintf(args_copy, sizeof(args_copy), "%s", args_ptr);

			uint8_t temp_data[DATA_SIZE];
			size_t cur_size = data_offset;
			size_t capacity = DATA_SIZE;

			assembler_error derr = parse_data_directive(temp_data, directive, args_copy, p, &cur_size, capacity);

			if (derr != ASSEMBLER_OK) {
				fclose(fp);
				return derr;
			}

			data_offset = (uint32_t)cur_size;
		} else {
			log_msg(LOG_ERROR, "Unknown segment name while scanning: %s", segment);
			fclose(fp);
			return ASSEMBLER_ERR;
		}
	}

	ctx->segments[SEGMENT_TEXT].size = text_offset;
	ctx->segments[SEGMENT_DATA].size = data_offset;

	fclose(fp);

	return ASSEMBLER_OK;
}
