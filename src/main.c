#include "opcodes.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 256
#define KEYWORD_SIZE 6

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stdout, "Usage: %s <FILE.s>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char line[BUF_SIZE];
	// Open file
	char opcode[KEYWORD_SIZE];
	// char reg1[KEYWORD_SIZE];
	// char reg2[KEYWORD_SIZE];
	// char reg3[KEYWORD_SIZE];

	/*uint8_t opcodes[OPCODES_NUM];
	for (int i = 0; i < OPCODES_NUM; i++) {
		opcodes[i] = 0;
	}*/

	FILE *f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Error: unknown file\n");
		return -1;
	}
    instruction_t instructions[OPCODES_NUM];
    build_instructions("../test/opcode_definition.s", instructions, OPCODES_NUM);
    /*
	while (fgets(line, sizeof(line), f)) {
		sscanf(line, "%s\n", opcode);
		opcodes[sdbm(opcode) % (OPCODES_NUM)]++;
	}

	for (int i = 0; i < OPCODES_NUM; i++) {
		if(opcodes[i] > 1){
            printf("Multiple occurrencies at: %d\n", opcodes[i]);
        }
	}
	// Read each line
	// Parse each line
	// Write output file
    */
	fclose(f);

	return EXIT_SUCCESS;
}
