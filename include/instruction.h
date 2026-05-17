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

#ifndef ASSEMBLER_INSTRUCTION_H
#define ASSEMBLER_INSTRUCTION_H

#include "error.h"

#include <stdint.h>

#define REGISTER_LEN 5
#define NAME_LEN 64

#define INT12_MAX ((1 << 11) - 1)
#define INT12_MIN (-(1 << 11))

#define INT13_MAX ((1 << 12) - 1)
#define INT13_MIN (-(1 << 12))

#define INT21_MAX ((1 << 20) - 1)
#define INT21_MIN (-(1 << 20))

/*
 * LUI accepts any values which can be encoded in 20 bits, so we can unify two
 * ranges to determine our MIN and MAX acceptable value:
 * - unsigned: 0 to 1048576 (2^20)
 * - signed: -524288 (-2^19) to 524287 (2^19 - 1)
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
assembler_error assemble_file(const char *filename);

#endif
