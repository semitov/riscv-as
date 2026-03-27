#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Assuming key is NULL terminated */
/*static unsigned int hash(const char *key) {
	unsigned int tot = 0;
	size_t len = strlen(key);

	for (size_t i = 0; i < len; ++i) {
		tot += (int)key[i];
	}

	return tot % HASHMAP_SIZE;
}*/

hashmap_s *hm_init() {
	hashmap_s *hashmap = calloc(HASHMAP_SIZE, sizeof *hashmap);
	if (!hashmap) {
		return NULL;
	}

	return hashmap;
}

/*int hm_set(hashmap_s *hashmap, char *key, instruction_s value) {
	if (!key) {
		return EXIT_FAILURE;
	}

	unsigned int index = hash(key, strlen(key));
	snprintf(hashmap[index].key, sizeof hashmap[index].key, "%s", key);
	hashmap[index].value = value;
	printf("[DEBUG/hm_set] Key: %s Type: %c\n",key, value.type);
	return EXIT_SUCCESS;
}*/

/*instruction_s *hm_get(hashmap_s *hashmap, char *key) {
	if (!hashmap || !key) {
		return NULL;
	}

	unsigned int index = hash(key, strlen(key));
	if (!strcmp(hashmap[index].key, "")) {
		return NULL;
	}
	printf("[DEBUG/hm_get] Key: %s Type: %c\n",key, hashmap[index].value.type);
	return &hashmap[index].value;
}*/

/*void hm_free(hashmap_s *hashmap) {
	if (hashmap) {
		free(hashmap);
	}
}
*/
