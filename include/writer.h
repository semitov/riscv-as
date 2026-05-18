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

/*
 * @TODO: ELF binary writer.
 */

#ifndef ASSEMBLER_WRITER_H
#define ASSEMBLER_WRITER_H

#include "error.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Produces an ELF (32 bits)
 *
 * @param filename Output file.
 * @param code Bytes array.
 * @param code_len Array length.
 */
assembler_error writer32(const char *filename, uint8_t *code, size_t code_len, uint32_t base_vaddr);

#endif
