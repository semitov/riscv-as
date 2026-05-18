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

#include <argp.h>
#include <stdlib.h>

const char *argp_program_version = "0.2";
const char doc[] = "semitov-riscv-as";

static struct argp_option options[] = {
	{"compile", 'c', "FILE", 0, "Compile an assembly file", -1},
	{"output", 'o', "FILE", 0, "Binary output file", -1},
	{"pk", 'p', 0, 0, "Set the correct virtual memory base for riscv-pk", -1},
	{0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	arguments_s *args = state->input;

	switch (key) {
		case 'c': {
			args->infile = arg;
			break;
		}
		case 'o': {
			args->outfile = arg;
			break;
		}
		case 'p': {
			args->base_vaddr = 0x00010000;
			break;
		}
		default: {
			return ARGP_ERR_UNKNOWN;
		}
	}
	return 0;
}

static struct argp argp = {options, parse_opt, 0, doc, NULL, NULL, NULL};

arguments_s *argparse(int argc, char **argv) {
	arguments_s *arguments = calloc(1, sizeof *arguments);
	if (!arguments) {
		return NULL;
	}

	/* Default values */
	arguments->infile = NULL;
	arguments->outfile = NULL;
	/* bare metal base virtual address */
	arguments->base_vaddr = 0x80000000;

	argp_parse(&argp, argc, argv, 0, 0, arguments);

	return arguments;
}

void argparse_free(arguments_s *arguments) {
	if (arguments) {
		free(arguments);
	}
}
