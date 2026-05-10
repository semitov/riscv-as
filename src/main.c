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
