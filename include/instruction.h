#ifndef ASSEMBLER_INSTRUCTION_H
#define ASSEMBLER_INSTRUCTION_H

#include "hash.h"

/**
 * @brief Loads the instruction into the hashmap.
 *
 * @param filename The definitions schema file path.
 * @param hashamp The hashmap.
 *
 * @note Must be called before using the assembler.
 */
// int load_instructions(const char *filename, hashmap_s *hashmap);
int assemble_file(const char *filename);

#endif
