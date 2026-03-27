#ifndef ASSEMBLER_COMMON_H
#define ASSEMBLER_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define INSTRUCTIONS_COUNT 40

typedef struct instruction {
	char *name;
	uint8_t opcode;
	uint8_t funct3;
	uint8_t funct7;
	char type;
} instruction_s;

#endif
