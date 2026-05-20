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
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Returns true if the system is little endian, false otherwise.
 *
 * @return true or false.
 */
bool is_little_endian(void);

// @TODO: remove this function later
void print_code(const uint8_t *code, size_t code_len);

/**
 * @brief Returns true if the line is empty or a comment-only line.
 *
 * @param str The line to inspect.
 * @return true if the line should be skipped.
 */
bool is_ignorable_line(const char *str);

char *string_trim_left(char *s);
char *string_trim_right(char *s);
char *string_trim(char *s);

#endif
