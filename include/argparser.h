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
 * Command-line argument parser.
 */

#ifndef ASSEMBLER_ARGPARSER_H
#define ASSEMBLER_ARGPARSER_H

#include <stdint.h>

/**
 * @brief Holds the parsed CLI arguments for the assembler.
 */
typedef struct arguments {
	char *infile;		/**< Path to the source file to assemble */
	char *outfile;		/**< Path to the output file binary */
	uint32_t base_vaddr; /* riscv-pk uses different virtual address (user-space) */
} arguments_s;

/**
 * @brief Parses CLI arguments into an arguments_s struct.
 *
 * @return Pointer to a heap-allocated arguments_s, or NULL on failure.
 *
 * @note The caller is responsible for freeing the returned pointer via argparse_free().
 */
arguments_s *argparse(int argc, char **argv);

/**
 * @brief Frees an arguments_s struct.
 *
 * @param arguments Pointer to the struct to free.
 */
void argparse_free(arguments_s *arguments);

#endif
