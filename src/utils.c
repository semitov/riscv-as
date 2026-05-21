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

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool is_little_endian(void) {
	int32_t i = 0x1;
	uint8_t *p = (uint8_t *)&i;

	return *p == 0x1;
}

void print_code(const uint8_t *code, size_t code_len) {
	if (!code) {
		return;
	}

	for (size_t i = 0; i < code_len; ++i) {
		fprintf(stdout, "%02X ", code[i]);
	}
	fprintf(stdout, "\n");
}

bool is_ignorable_line(const char *str) {
	if (!str) {
		return true;
	}

	while (*str != '\0' && isspace((unsigned char)*str)) {
		str++;
	}

	return *str == '\0' || *str == '#';
}

char *string_trim_left(char *s) {
	if (!s) {
		return NULL;
	}

	while (*s != '\0' && isspace((unsigned char)*s)) {
		s++;
	}

	return s;
}

char *string_trim_right(char *s) {
	if (!s) {
		return NULL;
	}

	size_t len = strlen(s);
	while (len > 0 && isspace((unsigned char)s[len - 1])) {
		len--;
	}
	s[len] = '\0';

	return s;
}

char *string_trim(char *s) {
	if (!s || *s == '\0') {
		return s;
	}

	return string_trim_right(string_trim_left(s));
}
