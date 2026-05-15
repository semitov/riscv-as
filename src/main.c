/*  SemiTOV-Assembler, Small RISC-V Assembler.
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
#include "instruction.h"

#include <stdint.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	arguments_s *arguments = {0};
	int ret = 0;

	arguments = argparse(argc, argv);
	if (!arguments) {
		return EXIT_FAILURE;
	}

	log_msg(LOG_INFO, "compiling %s ...", arguments->file);

	ret = assemble_file(arguments->file);
	if (ret != EXIT_SUCCESS) {
		log_msg(LOG_ERROR, "%s", "assemble_file() failed");
		argparse_free(arguments);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
