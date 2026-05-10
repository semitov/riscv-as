/*
 * Command-line argument parser.
 */

#ifndef ASSEMBLER_ARGPARSER_H
#define ASSEMBLER_ARGPARSER_H

/**
 * @brief Holds the parsed CLI arguments for the assembler.
 */
typedef struct arguments {
	char *file; /**< Path to the source file to assemble */
} arguments_s;

/**
 * @brief Parses CLI arguments into an arguments_s struct.
 *
 * @return Pointer to a heap-allocated arguments_s, or NULL on failure.
 *
 * @note The caller is responsible for freeing the returned pointer via argparse_free().
 */
arguments_s *argparse(int argc, char **argv);

/**
 * @brief Frees an arguments_s struct.
 *
 * @param arguments Pointer to the struct to free.
 */
void argparse_free(arguments_s *arguments);

#endif
