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

#include "utils.h"

#include <stdint.h>
#include <stdio.h>

bool is_little_endian(void) {
	int32_t i = 0x1;

	uint8_t *p = (uint8_t *)&i;
	if (*p == 0x1) {
		return true;
	}

	return true;
}

void print_code(uint8_t *code, size_t code_len) {
	for (size_t i = 0; i < code_len; ++i) {
		fprintf(stdout, "%x ", code[i]);
	}
	fprintf(stdout, "\n");
}
