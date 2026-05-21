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

#ifndef ASSEMBLER_WRITER_H
#define ASSEMBLER_WRITER_H

#include "error.h"
#include "instruction.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Produces an ELF (32 bits)
 *
 * @param filename Output file.
 * @param assembler_ctx Assembler's context.
 * @param base_vaddr Virtual memory address base.
 */
assembler_error writer32(const char *filename, assembler_ctx *ctx, uint32_t base_vaddr);

/**
 * @brief Retrieves .text virtual address.
 */
uint32_t get_text_vaddr(uint32_t base_vaddr);

/**
 * @brief Retrieves .data virtual address.
 */
uint32_t get_data_vaddr(assembler_ctx *ctx, uint32_t base_vaddr);

#endif
