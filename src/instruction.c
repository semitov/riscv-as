#include "instruction.h"

#include "debug.h"
#include "hash.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* S-Type Helpers */
#define GET_IMM_0_4(imm)		( imm & 0x1F )
#define GET_IMM_5_11(imm)		( (imm & 0xFE0) >> 5 )
// Get from assembled instruction
#define GET_FI_IMM_0_4(instr)	( instr & 0xF80 )
#define GET_FI_IMM_5_11(instr)	( instr & 0x7F000000 )
#define GET_FI_FUNCT3(instr)	( instr & 0x7000 )
#define GET_FI_RS1(instr)		( instr & 0xF8000 )
#define GET_FI_RS2(instr)		( instr & 0x1F00000 )
/* Return 1 if the string is made of only whitespaces */
static int is_only_ws(const char* str){
    while(*str!='\0'){
        if( !isspace(*str) )
            return 0;
        str++;
    }
    return 1;
}

/* Return 1 (true) if the opcode is a load/store one */
static int is_load_store(const char *op) {
	if (!op) {
		return 0;
	}

	return strcmp(op, "lb") == 0 || strcmp(op, "lh") == 0 || strcmp(op, "lw") == 0 || strcmp(op, "lbu") == 0 ||
		   strcmp(op, "lhu") == 0 || strcmp(op, "sb") == 0 || strcmp(op, "sh") == 0 || strcmp(op, "sw") == 0;
}

/**
 * @return 1 if the instruction is a pseudo-instruction, 0 otherwise.
 */
static int is_pseudo_instr(const char *name) {
	if (!name) {
		return 0;
	}

	return strcmp(name, "li") == 0 || strcmp(name, "mv") == 0 ||
		strcmp(name, "neg") == 0 || strcmp(name, "nop") == 0 ||
			strcmp(name, "not") == 0 || strcmp(name, "seqz") == 0 ||
				strcmp(name, "snez") == 0;
}

void expand_pseudo_instr(const instruction_s *instr, const char *lineBuf,
						 const instruction_s **out_instructions,
						 char out_lines[2][512]) {
	char rd[REGISTER_LEN], rs1[REGISTER_LEN], rs2[REGISTER_LEN];
	int64_t imm64 = 0x0;
	if (strcmp(instr->name,"li") == 0) {
		sscanf(lineBuf, "%*s %4[^,], %li[^#\n] ", rd, &imm64);
		if (imm64 < INT32_MIN || imm64 > INT32_MAX) {
			log_msg(LOG_ERROR, "Invalid immediate value in: %s\n"
					  "32 bit registers can't hold such value", lineBuf);
			exit(EXIT_FAILURE);
		}
		int32_t imm32 = (int32_t)imm64;
		if (imm32 >= INT12_MIN && imm32 <= INT12_MAX) {
			out_instructions[0] = find_instruction("addi", strlen("addi"));
			snprintf(out_lines[0], 512, "addi %s, zero, %d\n", rd, imm32);
		} else {
			int32_t lo = (imm32 << 20) >> 20;

			int32_t hi_20 = (imm32 - lo) >> 12;

			out_instructions[0] = find_instruction("lui", strlen("lui"));
			snprintf(out_lines[0], 512, "lui %s, %d\n", rd, hi_20);

			if (lo != 0) {
				out_instructions[1] = find_instruction("addi", strlen("addi"));
				snprintf(out_lines[1], 512, "addi %s, %s, %d\n", rd, rd, lo);
			} else {
				out_instructions[1] = NULL;
				out_lines[1][0] = '\0';
			}
		}

	} else if (strcmp(instr->name,"mv") == 0) {
		sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1);
		/* mv is implemented with 'addi rs, r1, 0' */
		out_instructions[0] = find_instruction("addi", strlen("addi"));
		snprintf(out_lines[0], 512, "addi %s, %s, 0\n", rd, rs1);
	} else if (strcmp(instr->name,"neg") == 0) {
		sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1);
		/* neg is implemented with 'sub rd, zero, rs1' */
		out_instructions[0] = find_instruction("sub", strlen("sub"));
		snprintf(out_lines[0], 512, "sub %s, zero, %s\n", rd, rs1);
	} else if (strcmp(instr->name,"nop") == 0) {
		/* nop is implemented with 'addi x0, x0, 0' */
		out_instructions[0] = find_instruction("addi", strlen("addi"));
		snprintf(out_lines[0], 512, "addi zero, zero, 0\n");
	} else if (strcmp(instr->name,"not") == 0) {
		sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1);
		/* not is implemented with 'xori rd, rs1, -1' */
		out_instructions[0] = find_instruction("xori", strlen("xori"));
		snprintf(out_lines[0], 512, "xori %s, %s, -1\n", rd, rs1);
	} else if (strcmp(instr->name,"ret") == 0) {
		log_msg(LOG_ERROR, "RET requires jalr whis is not implemented yet: %s", instr->name);
		exit(EXIT_FAILURE);
		/* ret is implemented with 'jalr x0, x1, 0' */
		out_instructions[0] = find_instruction("jalr", strlen("jalr"));
		snprintf(out_lines[0], 512, "jalr x0, x1, 0\n");
	} else if (strcmp(instr->name,"seqz") == 0) {
		sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs1);
		/* seqz is implemented with 'sltiu rd, rs1, 1' */
		out_instructions[0] = find_instruction("sltiu", strlen("sltiu"));
		snprintf(out_lines[0], 512, "sltiu %s, %s, 1\n", rd, rs1);
	} else if (strcmp(instr->name,"snez") == 0) {
		sscanf(lineBuf, "%*s %4[^,], %4[^#\n]", rd, rs2);
		/* snez is implemented with 'sltu rd, x0, rs2' */
		out_instructions[0] = find_instruction("sltu", strlen("sltu"));
		snprintf(out_lines[0], 512, "sltu %s, zero, %s\n", rd, rs2);
	} else {
		log_msg(LOG_ERROR, "Unknown pseudo-instruction: %s", instr->name);
		exit(EXIT_FAILURE);
	}
}

static uint8_t get_register(char *reg) {
	uint8_t regValue = 0x0;

	switch (reg[0]) {
		case 'x':
			sscanf(&reg[1], "%hhu", &regValue);
			if (regValue > 31) {
				log_msg(LOG_ERROR, "Invalid register: %s", reg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'a':
			uint8_t a = 0x0;
			sscanf(&reg[1], "%hhu", &a);
			if (a > 7) {
				log_msg(LOG_ERROR, "Invalid register: %s", reg);
				exit(EXIT_FAILURE);
			}
			/* a0-a7 -> x10-x17*/
			regValue = a + 10;
			break;
		case 's':
			if (reg[1] == 'p') {
				if (strcmp(reg, "sp") != 0) {
					log_msg(LOG_ERROR, "Unknown register: %s", reg);
					exit(EXIT_FAILURE);
				}
				regValue = 0x2;
			} else {
				uint8_t s = 0x0;
				sscanf(&reg[1], "%hhu", &s);
				if (s > 11) {
					log_msg(LOG_ERROR, "Invalid register: %s", reg);
					exit(EXIT_FAILURE);
				}
				/* s0/s1 -> x8/x9 */
				/* s2-11 -> x18-27 */
				regValue = s < 2 ? s + 8 : s + 16;
			}
			break;
		case 't':
			/* tp -> x4 */
			if (reg[1] == 'p') {
				if (strcmp(reg, "tp") != 0) {
					log_msg(LOG_ERROR, "Unknown register: %s", reg);
					exit(EXIT_FAILURE);
				}
				regValue = 0x4;
			} else {
				uint8_t t = 0x0;
				sscanf(&reg[1], "%hhu", &t);
				if (t > 6) {
					log_msg(LOG_ERROR, "Invalid register: %s", reg);
					exit(EXIT_FAILURE);
				}
				/* t0-2 -> x5-7 */
				/* t3-6 -> x28-31 */
				regValue = t < 3 ? t + 5 : t + 25;
			}
			break;
		case 'r':
			if (strcmp(reg, "ra") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				exit(EXIT_FAILURE);
			}
			/* ra (return address) reg (x1)*/
			regValue = 0x1;
			break;
		case 'g':
			if (strcmp(reg, "gp") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				exit(EXIT_FAILURE);
			}
			/* gp -> x3 */
			regValue = 0x3;
			break;
		case 'z':
			if (strcmp(reg, "zero") != 0) {
				log_msg(LOG_ERROR, "Unknown register: %s", reg);
				exit(EXIT_FAILURE);
			}
			/* zero -> x0 */
			regValue = 0x0;
			break;
		default:
			log_msg(LOG_ERROR, "Unknown register: %s", reg);
			exit(EXIT_FAILURE);
			break;
	}

	return regValue;
}
static int32_t encode(const instruction_s *instr, char *lineBuf, char *name){
    char rd[REGISTER_LEN] = {0}, rs1[REGISTER_LEN] = {0}, rs2[REGISTER_LEN] = {0};
	int16_t imm=0x0, imm2=0x0;
	int32_t imm32=0x0;
    int32_t res = 0x0;
    switch (instr->type) {
				case R_TYPE:
					sscanf(lineBuf, "%*s %4[^,], %4[^,], %4[^#\n]", rd, rs1, rs2);
					log_msg(LOG_DEBUG, "R-Type Read: %s %s %s", rd, rs1, rs2);
					res = (instr->funct7 << 25) | (get_register(rs2) << 20) | (get_register(rs1) << 15) |
						  (instr->funct3 << 12) | (get_register(rd) << 7) | instr->opcode;
					log_msg(LOG_DEBUG, "Write: %08x", res);
					break;
				case I_TYPE:
					/* Load opcodes have a specific syntax opcode, reg, offset(reg) */
					if (is_load_store(name)) {

						size_t res = sscanf(lineBuf, "%*s %4[^,], %hi(%4[^)])", rd, &imm, rs1);
						/* Probably an implicit zero offset immediate */
						if (res < 3) {
							sscanf(lineBuf, "%*s %4[^,], %4[^\n#]", rd, rs1);
							imm = 0x0; // Still needed this immediate
						}
					} else {
						sscanf(lineBuf, "%*s %4[^,], %4[^,], %hi[^#\n] ", rd, rs1, &imm);
					}
    				if (imm < INT12_MIN || imm > INT12_MAX) {
    					log_msg(LOG_ERROR, "Invalid immediate value: %s", lineBuf);
    					exit(EXIT_FAILURE);
    				}
					log_msg(LOG_DEBUG, "I-Type Read: %s, %s, %hi", rd, rs1, imm);
					res = imm << 20 | (get_register(rs1) << 15) | (instr->funct3 << 12) | (get_register(rd) << 7) |
						  instr->opcode;
					log_msg(LOG_DEBUG, "Write: %08x", res);
					break;
				case S_TYPE:
					/* Store opcodes have a specific syntax opcode, src, offset(dest) */
					res = sscanf(lineBuf, "%*s %4[^,], %hi(%4[^)])", rs2, &imm, rs1);
					/* Assuming an implicit zero offset immediate */
					if (res < 3) {
						sscanf(lineBuf, "%*s %4[^,], %4[^\n#]", rs2, rs1);
						imm = 0x0;
					}
    				if (imm < INT12_MIN || imm > INT12_MAX) {
    					log_msg(LOG_ERROR, "Invalid immediate value: %s", lineBuf);
    					exit(EXIT_FAILURE);
    				}
					log_msg(LOG_DEBUG, "S-Type Read: %hi, %s, %s", imm, rs1, rs2);
					res = ( GET_IMM_5_11(imm) << 25) | (get_register(rs2) << 20) | (get_register(rs1) << 15) | (instr->funct3 << 12) |
						  ( GET_IMM_0_4(imm) << 7) | instr->opcode;
					log_msg(LOG_DEBUG, "Write: %08x", res);
					break;
				case B_TYPE:
					sscanf(lineBuf, "%*s %hi, %4[^,], %4[^,], %hi[^#\n] ", &imm, rs1, rs2, &imm2);
					log_msg(LOG_DEBUG, "B-Type Read: %hi, %s, %s, %hi", imm, rd, rs1, imm2);
					res = (imm2 << 25) | (get_register(rs2) << 20) | (get_register(rs1) << 15) | (instr->funct3 << 12) |
						  (imm << 7) | instr->opcode;
					log_msg(LOG_DEBUG, "Write: %08x", res);
					break;
				case U_TYPE:
					sscanf(lineBuf, "%*s %4[^,], %i[^#\n] ", rd, &imm32);
    				if (imm32 < INT20_MIN || imm32 > INT20_MAX) {
    					log_msg(LOG_ERROR, "Invalid immediate value: %s", lineBuf);
    					exit(EXIT_FAILURE);
    				}
					log_msg(LOG_DEBUG, "U-Type Read: %s, %i", rd, imm32);
					res = (imm32 << 12) | (get_register(rd) << 7) | instr->opcode;
					log_msg(LOG_DEBUG, "Write: %08x", res);
					break;
				case J_TYPE:
					sscanf(lineBuf, "%*s %4[^,], %i[^#\n] ", rd, &imm32);
					log_msg(LOG_DEBUG, "J-Type Read: %s, %i", rd, imm32);
					// TODO Write correctly imm bytes order
					res = (imm << 12) | (get_register(rd) << 7) | instr->opcode;
					log_msg(LOG_DEBUG, "Write: %08x", res);
					break;
				default:
					break;
			}
	return res;
}
int assemble_file(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return EXIT_FAILURE;
	}
	char name[NAME_LEN];
	int32_t res = 0x0;
	const instruction_s *instr;
	char lineBuf[512];
	size_t counter = 0;
	while ((fgets(lineBuf, 512, fp))) {
		sscanf(lineBuf, " %s ", name);
		instr = find_instruction(name, strlen(name));
		if (instr == NULL) {
			log_msg(LOG_ERROR, "[error] unknown instruction: %s\n", name);
			break;
		}
		if( !is_only_ws(lineBuf) ){
			if (is_pseudo_instr(name)) {
				log_msg(LOG_INFO, "expanding pseudo-instruction: %s", name);
				/* In RV32I some pseudo-instructions could require two actual
				   instructions to be correctly executed */
				const instruction_s *pseudo_expansion[2] = {NULL, NULL};
				char expanded_lines[2][512] = {0};
				expand_pseudo_instr(instr, lineBuf, pseudo_expansion, expanded_lines);
				for (int i = 0; i < 2 && pseudo_expansion[i] != NULL; i++) {
					res = encode(pseudo_expansion[i], expanded_lines[i], pseudo_expansion[i]->name);
					printf("%02lx:\t%08x\t%s", counter, res, expanded_lines[i]);
					counter += 4;
				}
			} else {
			    log_msg(LOG_INFO, "fetching instruction: %s (%c TYPE)", name, instr->type);
				res = encode(instr, lineBuf, name);
				printf("%02lx:\t%08x\t%s", counter, res, lineBuf);
				counter += 4;
			}
		}
	}
	fclose(fp);

	return EXIT_SUCCESS;
}
