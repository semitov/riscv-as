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

#include "argparser.h"
#include "debug.h"
#include "error.h"
#include "instruction.h"
#include "utils.h"
#include "writer.h"

#include <stdint.h>
#include <stdlib.h>

void init_segments(segment *segments) {
	for (size_t i = 0; i < SEGMENTS_NUM; ++i) {
		segments[i].type = (segment_type)i;
		segments[i].vaddr = 0;
		segments[i].capacity = DATA_SIZE;
		segments[i].size = 0;
	}
}

int main(int argc, char **argv) {
	int exit_code = EXIT_FAILURE;
	arguments_s *arguments = NULL;

	// Initialize assembler segments
	segment assembler_ctx[SEGMENTS_NUM];
	init_segments(assembler_ctx);

	// Parse cli args
	arguments = argparse(argc, argv);
	if (!arguments) {
		return exit_code;
	}

	if (arguments->infile) {
		assembler_error err = assemble_file(arguments->infile, assembler_ctx);
		if (err != ASSEMBLER_OK) {
			log_msg(LOG_ERROR, "assemble_file() failed: %s", assembler_error_str(err));
			goto cleanup;
		}
	}

	// log some infos
	log_msg(LOG_DEBUG, "data: %ld/%ld", assembler_ctx[SEGMENT_DATA].size, assembler_ctx[SEGMENT_DATA].capacity);
	print_code(assembler_ctx[SEGMENT_DATA].data, assembler_ctx[SEGMENT_DATA].size);

	log_msg(LOG_DEBUG, "text: %ld/%ld", assembler_ctx[SEGMENT_TEXT].size, assembler_ctx[SEGMENT_TEXT].capacity);
	print_code(assembler_ctx[SEGMENT_TEXT].data, assembler_ctx[SEGMENT_TEXT].size);

	log_msg(LOG_DEBUG, "bss: %ld/%ld", assembler_ctx[SEGMENT_BSS].size, assembler_ctx[SEGMENT_BSS].capacity);
	print_code(assembler_ctx[SEGMENT_BSS].data, assembler_ctx[SEGMENT_BSS].size);

	if (assembler_ctx[SEGMENT_TEXT].size) {
		char *filename = NULL;

		if (arguments->outfile) {
			filename = arguments->outfile;
		} else {
			filename = "a.out";
		}

		// This is not the final function call
		assembler_error err = writer32(filename, assembler_ctx[SEGMENT_TEXT].data, assembler_ctx[SEGMENT_TEXT].size,
									   arguments->base_vaddr);
		if (err != ASSEMBLER_OK) {
			log_msg(LOG_ERROR, "writer32() failed: %s", assembler_error_str(err));
			goto cleanup;
		}
	}

	exit_code = EXIT_SUCCESS;

cleanup:
	argparse_free(arguments);
	return exit_code;
}
