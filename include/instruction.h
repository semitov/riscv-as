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
#include "symtable.h"

#include <stddef.h>
#include <stdint.h>

// We support 3 segments: .data, .text and .bss
#define REGISTER_LEN 5
#define NAME_LEN 64

#define SEGMENTS_NUM 3
#define OPCODE_LEN 4
#define DATA_SIZE OPCODE_LEN * 512

#define LINE_BUF_LEN 512

#define INT12_MAX ((1 << 11) - 1)
#define INT12_MIN (-(1 << 11))

#define INT13_MAX ((1 << 12) - 1)
#define INT13_MIN (-(1 << 12))

#define INT20_MAX ((1 << 20) - 1)
#define INT20_MIN (-(1 << 19))

#define INT21_MAX ((1 << 20) - 1)
#define INT21_MIN (-(1 << 20))

// Check if immediate is in specified boundaries
#define CHECK_IMMEDIATE(imm, MIN, MAX, buf)                                                                            \
	if (imm < MIN || imm > MAX) {                                                                                      \
		log_msg(LOG_ERROR, "Invalid immediate value: %s", buf);                                                        \
		return ASSEMBLER_INVALID_IMMEDIATE;                                                                            \
	}

// Check if immediate is in specified boundaries and it's last bit
#define CHECK_IMM_AND_LAST_B(imm, MIN, MAX, buf, instr_type)                                                           \
	if (imm < MIN || imm > MAX || (imm_long & 0x1) != 0) {                                                             \
		log_msg(LOG_ERROR, "Invalid %s immediate value: %s", instr_type, lineBuf);                                     \
		return ASSEMBLER_INVALID_IMMEDIATE;                                                                            \
	}

#define R_TYPE 'R'
#define I_TYPE 'I'
#define S_TYPE 'S'
#define B_TYPE 'B'
#define U_TYPE 'U'
#define J_TYPE 'J'
#define Z_TYPE 'Z'

#define ASSEMBLE_R_TYPE(instr, rs2, rs1, rd)                                                                           \
	(int32_t)(((uint32_t)instr->funct7 << 25) | ((uint32_t)rs2 << 20) | ((uint32_t)rs1 << 15) |                        \
			  ((uint32_t)instr->funct3 << 12) | ((uint32_t)rd << 7) | instr->opcode);

#define ASSEMBLE_I_TYPE(instr, rs1, imm, rd)                                                                           \
	(int32_t)((((uint32_t)imm & 0xFFFu) << 20) | ((uint32_t)rs1 << 15) | ((uint32_t)instr->funct3 << 12) |             \
			  ((uint32_t)rd << 7) | instr->opcode);

#define ASSEMBLE_S_TYPE(instr, rs1, rs2, imm)                                                                          \
	(int32_t)((((imm >> 5) & 0x7Fu) << 25) | ((uint32_t)rs2 << 20) | ((uint32_t)rs1 << 15) |                           \
			  ((uint32_t)instr->funct3 << 12) | ((imm & 0x1Fu) << 7) | instr->opcode);

#define ASSEMBLE_B_TYPE(instr, rs1, rs2, imm)                                                                          \
	(int32_t)((((imm >> 12) & 0x1u) << 31) | (((imm >> 5) & 0x3Fu) << 25) | ((uint32_t)rs2 << 20) |                    \
			  ((uint32_t)rs1 << 15) | ((uint32_t)instr->funct3 << 12) | (((imm >> 1) & 0xFu) << 8) |                   \
			  (((imm >> 11) & 0x1u) << 7) | instr->opcode);

#define ASSEMBLE_U_TYPE(instr, rd, imm)                                                                                \
	(int32_t)((((uint32_t)imm & 0xFFFFFu) << 12) | ((uint32_t)rd << 7) | instr->opcode);

#define ASSEMBLE_J_TYPE(instr, rd, imm)                                                                                \
	(int32_t)((((imm >> 20) & 0x1u) << 31) | (((imm >> 1) & 0x3FFu) << 21) | (((imm >> 11) & 0x1u) << 20) |            \
			  (((imm >> 12) & 0xFFu) << 12) | ((uint32_t)rd << 7) | instr->opcode);

#define ASSEMBLE_Z_TYPE(instr)                                                                                         \
	(int32_t)(((uint32_t)instr->funct7 << 25) | ((uint32_t)instr->funct3 << 12) | instr->opcode);

typedef enum segment_type {
	SEGMENT_DATA = 0,
	SEGMENT_TEXT = 1,
	SEGMENT_BSS = 2,
} segment_type;

typedef struct segment {
	segment_type type;
	uint8_t data[DATA_SIZE];
	size_t size;
	size_t capacity;
} segment;

typedef struct instruction {
	char *name;
	uint8_t opcode;
	uint8_t funct3;
	uint8_t funct7;
	char type;
} instruction;

typedef struct assembler_ctx {
	segment segments[SEGMENTS_NUM];
	symtable table;
	uint32_t base_vaddr;
} assembler_ctx;

/**
 * @brief Assemble the file.
 *
 * @param filename File to assemble.
 * @param[out] ctx Assembler's ctx.
 */
assembler_error assemble_file(const char *filename, assembler_ctx *ctx);

/**
 * @brief Scans the file to calculate labels addresses.
 *
 * @param filename File to scan.
 * @param ctx Assembler's context.
 */
assembler_error scan_labels(const char *filename, assembler_ctx *ctx);

#endif
