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

void test_handle_r_type_registers(void){
    uint8_t rs1_val, rs2_val, rd_val;
    int32_t res;
    instruction *ptr = &instr;
    char *registers[32] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10",
        "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20",
        "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"};
    char *aliases[32] = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3",
        "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
    for(size_t i = 0; i < 32; i++){
        for(size_t j = 0; j < 32; j++){
            for(size_t k = 0; k < 32; k++){
                TEST_ASSERT_EQUAL( 0 ,handle_r_type_instr(&instr, registers[i], &rs1_val, registers[j], &rs2_val, registers[k], &rd_val, &res));
                TEST_ASSERT_EQUAL( ASSEMBLE_R_TYPE(ptr, rs2_val, rs1_val, rd_val), res);

                TEST_ASSERT_EQUAL( 0 ,handle_r_type_instr(&instr, aliases[i], &rs1_val, aliases[j], &rs2_val, aliases[k], &rd_val, &res));
                TEST_ASSERT_EQUAL( ASSEMBLE_R_TYPE(ptr, rs2_val, rs1_val, rd_val), res);
            }
        }
    }
}

void test_handle_r_type_should_fail(void){
    uint8_t rs1_val, rs2_val, rd_val;
    int32_t res;
    char *rs1, *rs2, *rd;
    rs1 = "x32";
    rs2 = "a2";
    rd = "zero";
    TEST_ASSERT_EQUAL( ASSEMBLER_INVALID_REGISTER, handle_r_type_instr(&instr, rs1, &rs1_val, rs2, &rs2_val, rd, &rd_val, &res));
    TEST_ASSERT_EQUAL( ASSEMBLER_INVALID_REGISTER, handle_r_type_instr(&instr, rd, &rs1_val, rs2, &rs2_val, rs1, &rd_val, &res));
    TEST_ASSERT_EQUAL( ASSEMBLER_INVALID_REGISTER, handle_r_type_instr(&instr, rs2, &rs1_val, rs1, &rs2_val, rd, &rd_val, &res));
}
