#include "error.h"
#include <stddef.h>

#define ARR_SIZE(X) (sizeof(X) / sizeof(*X))

typedef struct message {
	assembler_error error;
	const char *message;
} message;

static const message messages[] = {
	{ASSEMBLER_OK, "OK"},
	{ASSEMBLER_ERR, "ERR"},
	{ASSEMBLER_REGISTER_VALUE, "32 bit registers can't hold such value"},
	{ASSEMBLER_JUMP_NOT_IMPLEMENTED, "jump is not implemented yet"},
	{ASSEMBLER_UNKNOWN_PSEUDO, "Unknown pseudo-instruction"},
	{ASSEMBLER_INVALID_REGISTER, "Invalid register"},
	{ASSEMBLER_INVALID_IMMEDIATE, "Invalid immediate value"},
	{ASSEMBLER_PARSE_ERROR, "Failed to parse instruction"},
	{ASSEMBLER_FILE_ERROR, "Failed to open input file"},
	{ASSEMBLER_UNKNOWN_INSTRUCTION, "Unknown instruction"},
	{ASSEMBLER_ELF_ERROR, "Error while producing elf binary"},
	{ASSEMBLER_NULL_ERROR, "NULL pointer"},
};

const char *assembler_error_str(assembler_error err) {
	for (size_t i = 0; i < ARR_SIZE(messages); ++i) {
		if (messages[i].error == err) {
			return messages[i].message;
		}
	}

	return "No error message found!";
}
