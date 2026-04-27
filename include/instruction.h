#ifndef ASSEMBLER_INSTRUCTION_H
#define ASSEMBLER_INSTRUCTION_H

#include <stdint.h>

#define REGISTER_LEN 5
#define NAME_LEN 64

#define INT12_MAX ((1 << 11) - 1)
#define INT12_MIN (-(1 << 11))

/*
 * LUI accepts any values which can be encoded in 20 bits, so we can unify two
 * ranges to determine our MIN and MAX acceptable value:
 * - unsigned: 0 to 1048576 (2^20)
 * - signed: -524288 (-2^19) to 524287 (2^19 - 1)
 *
 */
#define INT20_MAX ((1 << 20) - 1)
#define INT20_MIN (-(1 << 19))

#define R_TYPE 'R'
#define I_TYPE 'I'
#define S_TYPE 'S'
#define B_TYPE 'B'
#define U_TYPE 'U'
#define J_TYPE 'J'

typedef struct instruction {
	char *name;
	uint8_t opcode;
	uint8_t funct3;
	uint8_t funct7;
	char type;
} instruction_s;

/**
 * @brief Assemble the file.
 *
 * @param filename File to assemble.
 */
int assemble_file(const char *filename);

#endif
