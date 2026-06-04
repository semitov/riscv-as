#ifndef ASSEMBLER_SYMTABLE_H
#define ASSEMBLER_SYMTABLE_H

#define LABEL_LEN 64
#define MAX_SYMBOLS 128

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum symtype {
	SYM_SEGMENT_TEXT,
	SYM_SEGMENT_DATA,
} symtype;

typedef struct symbol {
	char name[LABEL_LEN];
	uint32_t offset;
	symtype type;
} symbol;

typedef struct symtable {
	symbol symbols[MAX_SYMBOLS];
	size_t count;
} symtable;

bool symtable_insert(symtable *table, const char *name, uint32_t address);
bool symtable_lookup(symtable *table, const char *name, uint32_t *out_address);

#endif
