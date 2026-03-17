#include "opcodes.h"

static uint8_t format_binary(char *s, size_t bits){
    uint8_t binValue = 0;
    for( int i = 0; i < bits; i++ )
        binValue = (binValue << 1) | (s[i] - '0');

    return binValue;
}

int build_instructions(const char *filename, instruction_t *instructions, size_t num){

    FILE *opcodeFile = fopen(filename, "r");

    if( opcodeFile == NULL ){
        printf("Error: unknown file\n");
        return -1;
    }

    char line[64];
    char t1[8], t2[8], t3[8];
    for( int i = 0; i < OPCODES_NUM; i++){
        if( !fgets(line, sizeof(line), opcodeFile) ){
            printf("Error: reached EOF\n");
            return -1;
        }
        sscanf(line, "%s %7[01] %3[01] %7[01]\n", instructions[i].name,
               t1,
               t2,
               t3);
        instructions[i].opcode = format_binary(t1, 7);
        instructions[i].funct3 = format_binary(t2, 3);
        instructions[i].funct7 = format_binary(t3, 7);

        printf("Opcode: %s\t%x\t%x\t%x\n",instructions[i].name,
               instructions[i].opcode,
               instructions[i].funct3,
               instructions[i].funct7
        );
    }

    fclose(opcodeFile);

    return 0;
}
