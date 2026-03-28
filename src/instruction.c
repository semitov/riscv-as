#include "instruction.h"

#include "hash.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t get_register(char *reg) {
	uint8_t regValue = 0x0;

	switch (reg[0]) {
		case 'x':
			sscanf(&reg[1], "%hhu", &regValue);
			break;
		case 'a':
			/* a0-a7 -> x10-x17*/
			regValue = (reg[1] - '0') + 10;
			break;
		case 's':
			if (reg[1] == 'p') {
				regValue = 0x2;
			} else {
				uint8_t t = 0x0;
				sscanf(&reg[1], "%hhu", &t);
				/* s0/s1 -> x8/x9 */
				/* s2-11 -> x18-27 */
				regValue = t < 2 ? t + 8 : t + 16;
			}
			break;
		case 't':
			/* tp -> x4 */
			if (reg[1] == 'p') {
				regValue = 0x4;
			} else {
				uint8_t t = 0x0;
				/* t0-2 -> x5-7 */
				/* t3-6 -> x28-31 */
				regValue = t < 3 ? t + 5 : t + 25;
			}
			break;
		case 'r':
			/* ra (return address) reg (x1)*/
			regValue = 0x1;
			break;
		case 'g':
			/* gp -> x3 */
			regValue = 0x3;
			break;
		default:
			break;
	}

	return regValue;
}

int assemble_file(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return EXIT_FAILURE;
	}

	char name[NAME_LEN];
	char rd[REGISTER_LEN], rs1[REGISTER_LEN], rs2[REGISTER_LEN];
	uint16_t imm, imm2;
	uint32_t imm32;
	const instruction_s *instr;

	while ((fscanf(fp, " %s ", name) != EOF)) {
		instr = find_instruction(name, strlen(name));
		if (instr == NULL) {
			fprintf(stderr, "[error] unknown instruction:\n%s\n", name);
			break;
		}

		fprintf(stdout, "[info] fetching instruction: %s (%c TYPE)\n", name, instr->type);

		switch (instr->type) {
			case R_TYPE:
				fscanf(fp, " %3[^,], %3[^,], %3[^,\n] ", rd, rs1, rs2);
				printf("R-Type Read: %s %s %s\n", rd, rs1, rs2);
				break;
			case I_TYPE:
				fscanf(fp, " %3[^,], %3[^,], %hu[^\n] ", rd, rs1, &imm);
				printf("I-Type Read: %s, %s, %hu\n", rd, rs1, imm);
				break;
			case S_TYPE:
				fscanf(fp, " %hu, %3[^,], %3[^,], %hu[^\n] ", &imm, rs1, rs2, &imm2);
				printf("S-Type Read: %hu, %s, %s, %hu\n", imm, rd, rs1, imm2);
				break;
			case B_TYPE:
				fscanf(fp, " %hu, %3[^,], %3[^,], %hu[^\n] ", &imm, rs1, rs2, &imm2);
				printf("B-Type Read: %hu, %s, %s, %hu\n", imm, rd, rs1, imm2);
				break;
			case U_TYPE:
				fscanf(fp, " %3[^,], %u[^\n] ", rd, &imm32);
				printf("U-Type Read: %s, %u\n", rd, imm32);
				break;
			case J_TYPE:
				fscanf(fp, " %3[^,], %u[^\n] ", rd, &imm32);
				printf("J-Type Read: %s, %u\n", rd, imm32);
				break;
			default:
				break;
		}
	}

	fclose(fp);

	return EXIT_SUCCESS;
}
