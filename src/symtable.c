#include "symtable.h"
#include <string.h>

bool symtable_insert(symtable *table, const char *name, uint32_t address) {
	if (table->count >= MAX_SYMBOLS) {
		return false;
	}

	for (size_t i = 0; i < table->count; ++i) {
		if (strcmp(name, table->symbols[i].name) == 0) {
			// Symbol already defined!
			return false;
		}
	}

	strncpy(table->symbols[table->count].name, name, LABEL_LEN - 1);
	table->symbols[table->count].offset = address;
	table->count++;

	return true;
}

bool symtable_lookup(symtable *table, const char *name, uint32_t *out_address) {
	for (size_t i = 0; i < table->count; ++i) {
		if (strcmp(name, table->symbols[i].name) == 0) {
			*out_address = table->symbols[i].offset;
			return true;
		}
	}

	return false;
}
