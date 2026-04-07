#include "argparser.h"
#include "instruction.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	arguments_s *arguments = {0};
	int ret = 0;

	arguments = argparse(argc, argv);
	if (!arguments) {
		return EXIT_FAILURE;
	}

	fprintf(stdout, "[info] compiling %s ...\n", arguments->file);

	ret = assemble_file(arguments->file);
	if (ret != EXIT_SUCCESS) {
		argparse_free(arguments);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
