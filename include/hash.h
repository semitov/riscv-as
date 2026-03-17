#ifndef ASSEMBLER_HASH_H
#define ASSEMBLER_HASH_H

#include <stdint.h>

#define OPCODES_NUM 512 // Way less but we avoid collisions.

unsigned long sdbm(char *keyword);

typedef struct hashMap_s{
    const uint8_t opcodes[OPCODES_NUM];

} hashMap;

#endif
