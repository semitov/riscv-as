#include "argparser.h"
#include "instruction.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	arguments_s *arguments = {0};

	arguments = argparse(argc, argv);
	if (!arguments) {
		goto cleanup;
	}

	fprintf(stdout, "[info] compiling %s ...\n", arguments->file);

	assemble_file(arguments->file);

	goto cleanup;

	return EXIT_SUCCESS;

cleanup:
	argparse_free(arguments);
}
