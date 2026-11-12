#include "error.h"
#include "instruction.h"
#include "unity.h"
static instruction instr;

void setUp(void){
    instr.name = "add";
    instr.opcode = 0x33;
    instr.funct3 = 0x0;
    instr.funct7 = 0x00;
    instr.type = 'R';
}
void tearDown(void){

}

void test_handle_r_type(void){
    uint8_t rs1_val, rs2_val, rd_val;
    printf("aaaa\n");
    char *rs1, *rs2, *rd;
    int32_t res;
    rs1 = "x1";
    rs2 = "x2";
    rd = "x3";
    TEST_ASSERT_EQUAL(0,handle_r_type_instr(&instr, rs1, &rs1_val, rs2, &rs2_val, rd, &rd_val, &res));
}
