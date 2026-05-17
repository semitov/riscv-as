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
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pseudo_expansion {
	const instruction_s *instructions[2];
	char lines[2][512];
	size_t count;
} pseudo_expansion;

static void pseudo_expansion_reset(pseudo_expansion *expansion) {
	if (!expansion) {
		return;
	}
	memset(expansion, 0, sizeof(*expansion));
}

static assembler_error pseudo_expansion_add(pseudo_expansion *expansion, const instruction_s *instr, const char *line) {
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
 * @brief Returns true if the line is empty or a comment-only line.
 *
 * @param str The line to inspect.
 * @return true if the line should be skipped.
 */
static bool is_ignorable_line(const char *str) {
	if (!str) {
		return true;
	}

	while (*str != '\0' && isspace((unsigned char)*str)) {
		str++;
	}

	return *str == '\0' || *str == '#';
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

static assembler_error expand_pseudo_instr(const instruction_s *instr, const char *lineBuf,
										   pseudo_expansion *out_expansion) {
	if (!instr || !lineBuf || !out_expansion) {
		return ASSEMBLER_PARSE_ERROR;
	}

	pseudo_expansion_reset(out_expansion);

	char rd[REGISTER_LEN] = {0};
	char rs1[REGISTER_LEN] = {0};
	char rs2[REGISTER_LEN] = {0};
	char line[512] = {0};
	int64_t imm64 = 0x0;

	if (strcmp(instr->name, "li") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %li", rd, &imm64) != 2) {
			log_msg(LOG_ERROR, "Failed to parse li: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}

		if (imm64 < INT32_MIN || imm64 > INT32_MAX) {
			log_msg(LOG_ERROR,
					"Invalid immediate value in: %s\n"
					"32 bit registers can't hold such value",
					lineBuf);
			return ASSEMBLER_REGISTER_VALUE;
		}

		int32_t imm32 = (int32_t)imm64;
		if (imm32 >= INT12_MIN && imm32 <= INT12_MAX) {
			const instruction_s *addi = find_instruction("addi", strlen("addi"));
			if (!addi) {
				return ASSEMBLER_UNKNOWN_INSTRUCTION;
			}
			snprintf(line, sizeof(line), "addi %s, zero, %d\n", rd, imm32);
			return pseudo_expansion_add(out_expansion, addi, line);
		}

		int32_t lo = (imm32 << 20) >> 20;
		int32_t hi_20 = (imm32 - lo) >> 12;

		const instruction_s *lui = find_instruction("lui", strlen("lui"));
		if (!lui) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, sizeof(line), "lui %s, %d\n", rd, hi_20);
		assembler_error err = pseudo_expansion_add(out_expansion, lui, line);
		if (err != ASSEMBLER_OK) {
			return err;
		}

		if (lo != 0) {
			const instruction_s *addi = find_instruction("addi", strlen("addi"));
			if (!addi) {
				return ASSEMBLER_UNKNOWN_INSTRUCTION;
			}
			snprintf(line, sizeof(line), "addi %s, %s, %d\n", rd, rd, lo);
			return pseudo_expansion_add(out_expansion, addi, line);
		}

		return ASSEMBLER_OK;
	}

	if (strcmp(instr->name, "mv") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse mv: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}
		/* mv is implemented with 'addi rs, r1, 0' */
		const instruction_s *addi = find_instruction("addi", strlen("addi"));
		if (!addi) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, sizeof(line), "addi %s, %s, 0\n", rd, rs1);
		return pseudo_expansion_add(out_expansion, addi, line);
	}

	if (strcmp(instr->name, "neg") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse neg: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}
		/* neg is implemented with 'sub rd, zero, rs1' */
		const instruction_s *sub = find_instruction("sub", strlen("sub"));
		if (!sub) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, sizeof(line), "sub %s, zero, %s\n", rd, rs1);
		return pseudo_expansion_add(out_expansion, sub, line);
	}

	if (strcmp(instr->name, "nop") == 0) {
		/* nop is implemented with 'addi x0, x0, 0' */
		const instruction_s *addi = find_instruction("addi", strlen("addi"));
		if (!addi) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, sizeof(line), "addi zero, zero, 0\n");
		return pseudo_expansion_add(out_expansion, addi, line);
	}

	if (strcmp(instr->name, "not") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse not: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}
		/* not is implemented with 'xori rd, rs1, -1' */
		const instruction_s *xori = find_instruction("xori", strlen("xori"));
		if (!xori) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, sizeof(line), "xori %s, %s, -1\n", rd, rs1);
		return pseudo_expansion_add(out_expansion, xori, line);
	}

	if (strcmp(instr->name, "ret") == 0) {
		/* jump is not implemented yet */
		log_msg(LOG_ERROR, "RET requires jalr whis is not implemented yet: %s", instr->name);
		return ASSEMBLER_JUMP_NOT_IMPLEMENTED;
	}

	if (strcmp(instr->name, "seqz") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1) != 2) {
			log_msg(LOG_ERROR, "Failed to parse seqz: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}
		/* seqz is implemented with 'sltiu rd, rs1, 1' */
		const instruction_s *sltiu = find_instruction("sltiu", strlen("sltiu"));
		if (!sltiu) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, sizeof(line), "sltiu %s, %s, 1\n", rd, rs1);
		return pseudo_expansion_add(out_expansion, sltiu, line);
	}

	if (strcmp(instr->name, "snez") == 0) {
		if (sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs2) != 2) {
			log_msg(LOG_ERROR, "Failed to parse snez: %s", lineBuf);
			return ASSEMBLER_PARSE_ERROR;
		}
		/* snez is implemented with 'sltu rd, x0, rs2' */
		const instruction_s *sltu = find_instruction("sltu", strlen("sltu"));
		if (!sltu) {
			return ASSEMBLER_UNKNOWN_INSTRUCTION;
		}
		snprintf(line, sizeof(line), "sltu %s, zero, %s\n", rd, rs2);
		return pseudo_expansion_add(out_expansion, sltu, line);
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
			/* a0-a7 -> x10-x17*/
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
			/* s0/s1 -> x8/x9 */
			/* s2-11 -> x18-27 */
			*out_value = reg_value < 2 ? reg_value + 8 : reg_value + 16;
			return ASSEMBLER_OK;
		case 't':
			/* tp -> x4 */
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
			/* t0-2 -> x5-7 */
			/* t3-6 -> x28-31 */
			*out_value = reg_value < 3 ? reg_value + 5 : reg_value + 25;
			return ASSEMBLER_OK;
		case 'r':
			if (strcmp(reg, "ra") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			/* ra (return address) reg (x1)*/
			*out_value = 0x1;
			return ASSEMBLER_OK;
		case 'g':
			if (strcmp(reg, "gp") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			/* gp -> x3 */
			*out_value = 0x3;
			return ASSEMBLER_OK;
		case 'z':
			if (strcmp(reg, "zero") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				return ASSEMBLER_INVALID_REGISTER;
			}
			/* zero -> x0 */
			*out_value = 0x0;
			return ASSEMBLER_OK;
		default:
			log_msg(LOG_ERROR, "Unknown register: %s", reg);
			return ASSEMBLER_INVALID_REGISTER;
	}
}

static assembler_error encode(const instruction_s *instr, const char *lineBuf, const char *name, int32_t *out_encoded) {
	if (!instr || !lineBuf || !name || !out_encoded) {
		return ASSEMBLER_PARSE_ERROR;
	}

	char rd[REGISTER_LEN] = {0};
	char rs1[REGISTER_LEN] = {0};
	char rs2[REGISTER_LEN] = {0};
	long imm_long = 0;
	int32_t imm = 0;
	int32_t res = 0;
	uint8_t rd_val = 0;
	uint8_t rs1_val = 0;
	uint8_t rs2_val = 0;
	assembler_error err = ASSEMBLER_OK;
	int scan_res = 0;
	uint32_t imm_u = 0;

	switch (instr->type) {
		case R_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %4[^,], %4[^#\n]", rd, rs1, rs2) != 3) {
				log_msg(LOG_ERROR, "Failed to parse R-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}
			log_msg(LOG_DEBUG, "R-Type Read: %s %s %s", rd, rs1, rs2);
			err = parse_register(rs2, &rs2_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			err = parse_register(rs1, &rs1_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			err = parse_register(rd, &rd_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			res = (int32_t)(((uint32_t)instr->funct7 << 25) | ((uint32_t)rs2_val << 20) | ((uint32_t)rs1_val << 15) |
							((uint32_t)instr->funct3 << 12) | ((uint32_t)rd_val << 7) | instr->opcode);
			log_msg(LOG_DEBUG, "Write: %08x", res);
			break;
		case I_TYPE:
			/* Load opcodes have a specific syntax opcode, reg, offset(reg) */
			if (is_load_store(name)) {
				scan_res = sscanf(lineBuf, "%*s %4[^,], %li(%4[^)])", rd, &imm_long, rs1);
				/* Probably an implicit zero offset immediate */
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
			if (imm_long < INT12_MIN || imm_long > INT12_MAX) {
				log_msg(LOG_ERROR, "Invalid immediate value: %s", lineBuf);
				return ASSEMBLER_INVALID_IMMEDIATE;
			}
			imm = (int32_t)imm_long;
			log_msg(LOG_DEBUG, "I-Type Read: %s, %s, %li", rd, rs1, imm_long);
			err = parse_register(rs1, &rs1_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			err = parse_register(rd, &rd_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			res = (int32_t)((((uint32_t)imm & 0xFFFu) << 20) | ((uint32_t)rs1_val << 15) |
							((uint32_t)instr->funct3 << 12) | ((uint32_t)rd_val << 7) | instr->opcode);
			log_msg(LOG_DEBUG, "Write: %08x", res);
			break;
		case S_TYPE:
			/* Store opcodes have a specific syntax opcode, src, offset(dest) */
			scan_res = sscanf(lineBuf, "%*s %4[^,], %li(%4[^)])", rs2, &imm_long, rs1);
			/* Assuming an implicit zero offset immediate */
			if (scan_res < 3) {
				scan_res = sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rs2, rs1);
				if (scan_res != 2) {
					log_msg(LOG_ERROR, "Failed to parse S-type: %s", lineBuf);
					return ASSEMBLER_PARSE_ERROR;
				}
				imm_long = 0;
			}
			if (imm_long < INT12_MIN || imm_long > INT12_MAX) {
				log_msg(LOG_ERROR, "Invalid immediate value: %s", lineBuf);
				return ASSEMBLER_INVALID_IMMEDIATE;
			}
			imm = (int32_t)imm_long;
			log_msg(LOG_DEBUG, "S-Type Read: %li, %s, %s", imm_long, rs1, rs2);
			err = parse_register(rs2, &rs2_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			err = parse_register(rs1, &rs1_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			imm_u = (uint32_t)imm;
			res = (int32_t)((((imm_u >> 5) & 0x7Fu) << 25) | ((uint32_t)rs2_val << 20) | ((uint32_t)rs1_val << 15) |
							((uint32_t)instr->funct3 << 12) | ((imm_u & 0x1Fu) << 7) | instr->opcode);
			log_msg(LOG_DEBUG, "Write: %08x", res);
			break;
		case B_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %4[^,], %li", rs1, rs2, &imm_long) != 3) {
				log_msg(LOG_ERROR, "Failed to parse B-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}
			if (imm_long < INT13_MIN || imm_long > INT13_MAX || (imm_long & 0x1) != 0) {
				log_msg(LOG_ERROR, "Invalid branch immediate value: %s", lineBuf);
				return ASSEMBLER_INVALID_IMMEDIATE;
			}
			imm = (int32_t)imm_long;
			log_msg(LOG_DEBUG, "B-Type Read: %s, %s, %li", rs1, rs2, imm_long);
			err = parse_register(rs1, &rs1_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			err = parse_register(rs2, &rs2_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			imm_u = (uint32_t)imm;
			res = (int32_t)((((imm_u >> 12) & 0x1u) << 31) | (((imm_u >> 5) & 0x3Fu) << 25) |
							((uint32_t)rs2_val << 20) | ((uint32_t)rs1_val << 15) | ((uint32_t)instr->funct3 << 12) |
							(((imm_u >> 1) & 0xFu) << 8) | (((imm_u >> 11) & 0x1u) << 7) | instr->opcode);
			log_msg(LOG_DEBUG, "Write: %08x", res);
			break;
		case U_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %li", rd, &imm_long) != 2) {
				log_msg(LOG_ERROR, "Failed to parse U-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}
			if (imm_long < INT20_MIN || imm_long > INT20_MAX) {
				log_msg(LOG_ERROR, "Invalid immediate value: %s", lineBuf);
				return ASSEMBLER_INVALID_IMMEDIATE;
			}
			imm = (int32_t)imm_long;
			log_msg(LOG_DEBUG, "U-Type Read: %s, %li", rd, imm_long);
			err = parse_register(rd, &rd_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			res = (int32_t)((((uint32_t)imm & 0xFFFFFu) << 12) | ((uint32_t)rd_val << 7) | instr->opcode);
			log_msg(LOG_DEBUG, "Write: %08x", res);
			break;
		case J_TYPE:
			if (sscanf(lineBuf, "%*s %4[^,], %li", rd, &imm_long) != 2) {
				log_msg(LOG_ERROR, "Failed to parse J-type: %s", lineBuf);
				return ASSEMBLER_PARSE_ERROR;
			}
			if (imm_long < INT21_MIN || imm_long > INT21_MAX || (imm_long & 0x1) != 0) {
				log_msg(LOG_ERROR, "Invalid jump immediate value: %s", lineBuf);
				return ASSEMBLER_INVALID_IMMEDIATE;
			}
			imm = (int32_t)imm_long;
			log_msg(LOG_DEBUG, "J-Type Read: %s, %li", rd, imm_long);
			err = parse_register(rd, &rd_val);
			if (err != ASSEMBLER_OK) {
				return err;
			}
			imm_u = (uint32_t)imm;
			res = (int32_t)((((imm_u >> 20) & 0x1u) << 31) | (((imm_u >> 1) & 0x3FFu) << 21) |
							(((imm_u >> 11) & 0x1u) << 20) | (((imm_u >> 12) & 0xFFu) << 12) | ((uint32_t)rd_val << 7) |
							instr->opcode);
			log_msg(LOG_DEBUG, "Write: %08x", res);
			break;
		default:
			log_msg(LOG_ERROR, "Unknown instruction type: %c", instr->type);
			return ASSEMBLER_ERR;
	}

	*out_encoded = res;
	return ASSEMBLER_OK;
}

assembler_error assemble_file(const char *filename) {
	if (!filename) {
		return ASSEMBLER_FILE_ERROR;
	}

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		log_msg(LOG_ERROR, "Failed to open file: %s", filename);
		return ASSEMBLER_FILE_ERROR;
	}

	char name[NAME_LEN] = {0};
	const instruction_s *instr = NULL;
	char lineBuf[512];
	size_t counter = 0;
	assembler_error err = ASSEMBLER_OK;

	while (fgets(lineBuf, sizeof(lineBuf), fp)) {
		if (is_ignorable_line(lineBuf)) {
			continue;
		}
		if (sscanf(lineBuf, " %63s ", name) != 1) {
			log_msg(LOG_ERROR, "Failed to parse instruction name: %s", lineBuf);
			err = ASSEMBLER_PARSE_ERROR;
			break;
		}
		instr = find_instruction(name, strlen(name));
		if (!instr) {
			log_msg(LOG_ERROR, "Unknown instruction: %s", name);
			err = ASSEMBLER_UNKNOWN_INSTRUCTION;
			break;
		}

		if (instr->type == 'P') {
			log_msg(LOG_INFO, "expanding pseudo-instruction: %s", name);
			/* In RV32I some pseudo-instructions could require two actual
			   instructions to be correctly executed */
			pseudo_expansion expansion;
			err = expand_pseudo_instr(instr, lineBuf, &expansion);
			if (err != ASSEMBLER_OK) {
				break;
			}
			for (size_t i = 0; i < expansion.count; i++) {
				int32_t encoded = 0;
				err = encode(expansion.instructions[i], expansion.lines[i], expansion.instructions[i]->name, &encoded);
				if (err != ASSEMBLER_OK) {
					break;
				}
				printf("%02lx:\t%08x\t%s", (unsigned long)counter, encoded, expansion.lines[i]);
				counter += 4;
			}
			if (err != ASSEMBLER_OK) {
				break;
			}
		} else {
			log_msg(LOG_INFO, "fetching instruction: %s (%c TYPE)", name, instr->type);
			int32_t encoded = 0;
			err = encode(instr, lineBuf, name, &encoded);
			if (err != ASSEMBLER_OK) {
				break;
			}
			printf("%02lx:\t%08x\t%s", (unsigned long)counter, encoded, lineBuf);
			counter += 4;
		}
	}

	fclose(fp);
	return err;
}
