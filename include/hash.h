#ifndef ASSEMBLER_HASH_H
#define ASSEMBLER_HASH_H

#include "common.h"

#define HASHMAP_SIZE 64
#define INSTRUCTION_NAME_MAX 8

typedef struct hashmap {
	char key[INSTRUCTION_NAME_MAX];
	instruction_s value;
} hashmap_s;

const struct instruction *in_word_set(register const char *str, register size_t len);

/**
 * @brief Initializes a new hashmap.
 *
 * @return The pointer of the new initialized hashmap, otherwise NULL.
 *
 * @note Remember to free.
 */

// hashmap_s *hm_init();

/**
 * @brief Inserts or updates an element in the hashmap.
 *
 * @param hashmap The hashmap.
 * @param key The key to add/modify.
 * @param value The value to set.
 *
 * @return 0 on success, -1 on failure.
 */
// int hm_set(hashmap_s *hashmap, char *key, instruction_s value);

/**
 * @brief Retrieves an element from the hashmap.
 *
 * @param hashmap The hashmap.
 * @param key The key to find.
 *
 * @return The pointer to the instruction if found, otherwise NULL.
 */
// instruction_s *hm_get(hashmap_s *hashmap, char *key);

/**
 * @brief Clears the memory freeing the hashmap.
 *
 * @param hashmap The hashmap.
 */
// void hm_free(hashmap_s *hashmap);

#endif
