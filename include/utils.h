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

#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool is_little_endian(void);
void print_code(uint8_t *code, size_t code_len);

#endif
