#include "argparser.h"
#include "hash.h"
#include "instruction.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	arguments_s *arguments = {0};
	// hashmap_s *hashmap = {0};

	arguments = argparse(argc, argv);
	if (!arguments) {
		goto cleanup;
	}

	// hashmap = hm_init();
	/*if (!hashmap) {
		goto cleanup;
	}*/

	fprintf(stdout, "[info] loading definitions schema from %s ...\n", arguments->definitions_schema);
	fprintf(stdout, "[info] compiling %s ...\n", arguments->file);

	/* Load definitions into instructions hashmap */
	// load_instructions(arguments->definitions_schema, hashmap);

	assemble_file(arguments->file);
	/* Retrieve some instructions */
	/*instruction_s *add = hm_get(hashmap, "add");
	if (!add) {
		fprintf(stderr, "[error] 'add' not found.\n");
	} else {
		fprintf(stdout, "[debug] 'add': %02x, %02x, %02x\n", add->opcode, add->funct3, add->funct7);
	}*/

	goto cleanup;

	return EXIT_SUCCESS;

cleanup:
	argparse_free(arguments);
	// hm_free(hashmap);
	return EXIT_FAILURE;
}
