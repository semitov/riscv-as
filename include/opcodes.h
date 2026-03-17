#ifndef OPCODES_H_
#define OPCODES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#define OPCODES_NUM 40

typedef struct instruction_s{
    char name[8];
    uint8_t opcode;
    uint8_t funct3;
    uint8_t funct7;

} instruction_t;

/* Must be called before using the assembler */
int build_instructions(const char *filename, instruction_t *instructions, size_t num);

#endif // OPCODES_H_
