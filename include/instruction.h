#ifndef ASSEMBLER_INSTRUCTION_H
#define ASSEMBLER_INSTRUCTION_H

#include <stdint.h>

#define REGISTER_LEN 4
#define NAME_LEN 64

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
