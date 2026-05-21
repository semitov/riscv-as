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
#include "writer.h"

#include <stdint.h>
#include <stdlib.h>

void init_segments(segment *segments) {
	for (size_t i = 0; i < SEGMENTS_NUM; ++i) {
		segments[i].type = (segment_type)i;
		segments[i].capacity = DATA_SIZE;
		segments[i].size = 0;
	}
}

int main(int argc, char **argv) {
	int exit_code = EXIT_FAILURE;
	arguments_s *arguments = NULL;
	assembler_error err = ASSEMBLER_OK;

	// Initialize assembler context
	assembler_ctx assembler_ctx = {0};
	init_segments(assembler_ctx.segments);

	// Parse cli args
	arguments = argparse(argc, argv);
	if (!arguments) {
		return exit_code;
	}

	assembler_ctx.base_vaddr = arguments->base_vaddr;

	if (arguments->infile) {
		// First scan to calculate label address
		err = scan_labels(arguments->infile, &assembler_ctx);
		if (err != ASSEMBLER_OK) {
			log_msg(LOG_ERROR, "scan_labels() failed: %s", assembler_error_str(err));
			goto cleanup;
		}

		// Assemble file
		err = assemble_file(arguments->infile, &assembler_ctx);
		if (err != ASSEMBLER_OK) {
			log_msg(LOG_ERROR, "assemble_file() failed: %s", assembler_error_str(err));
			goto cleanup;
		}
	}

	if (assembler_ctx.segments[SEGMENT_TEXT].size) {
		char *filename = NULL;

		if (arguments->outfile) {
			filename = arguments->outfile;
		} else {
			filename = "a.out";
		}

		assembler_error err = writer32(filename, &assembler_ctx, arguments->base_vaddr);
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
