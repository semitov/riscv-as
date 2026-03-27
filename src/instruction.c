#include "instruction.h"
#include "common.h"
#include "hash.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define OPCODE_MAX 8

static uint8_t format_binary(char *s, size_t bits) {
	uint8_t binValue = 0;
	for (int i = 0; i < bits; i++) {
		binValue = (binValue << 1) | (s[i] - '0');
	}

	return binValue;
}

static uint8_t get_register(char *reg) {
	uint8_t regValue = 0x0;
	switch (reg[0]) {
		case 'x':
			sscanf(&reg[1], "%hhd", &regValue);
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
				sscanf(&reg[1], "%hhd", &t);
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

	// char name[INSTRUCTION_NAME_MAX];
	char name[64];
	FILE *fp = fopen(filename, "r");
	char rd[4], rs1[4], rs2[4];
	uint16_t imm, imm2;
	uint32_t imm32;
	const instruction_s *instr;
	while ((fscanf(fp, "%s ", name) != EOF)) {
		instr = in_word_set(name, strlen(name));
		if (instr == NULL) {
			printf("[ERROR] Unknown instruction %s\n", name);
			break;
		}
		printf("Fetching instruction: %s with type: %c\n", name, instr->type);
		switch (instr->type) {
			case 'R':
				fscanf(fp, "%3[^,], %3[^,], %3[^,]", rd, rs1, rs2);
				printf("R-Type Read: %s %s %s\n", rd, rs1, rs2);
				break;
			case 'I':
				fscanf(fp, "%3[^,], %3[^,], %hd", rd, rs1, &imm);
				printf("I-Type Read: %s, %s, %hd\n", rd, rs1, imm);
				break;
			case 'S':
				fscanf(fp, "%hd, %3[^,], %3[^,], %hd", &imm, rs1, rs2, &imm2);
				printf("S-Type Read: %hd, %s, %s, %hd\n", imm, rd, rs1, imm2);
				break;
			case 'B':
				fscanf(fp, "%hd, %3[^,], %3[^,], %hd", &imm, rs1, rs2, &imm2);
				printf("B-Type Read: %hd, %s, %s, %hd\n", imm, rd, rs1, imm2);
				break;
			case 'U':
				fscanf(fp, "%3[^,], %d", rd, &imm32);
				printf("U-Type Read: %s, %d\n", rd, imm32);
				break;
			case 'J':
				fscanf(fp, "%3[^,], %d", rd, &imm32);
				printf("J-Type Read: %s, %d\n", rd, imm32);
				break;
			default:
				break;
		}
	}
	fclose(fp);
	return 0;
}
