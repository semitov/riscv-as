li t0, 100000
li t1, 100352
li t2, -4096
li t3, 2147483647
li t4, -2147483648
mv gp, zero
nop
neg t3, t1
not t4, s4
seqz a7, t6
snez a4, gp
li tp, -0x800

auipc a4, 0x12345
auipc a4, 74565
lui t0, 524288
lui t0, -524288
lui t0, 524289
lui t0, -524287
lui t0, 524287
lui t0, 1048575
lui t0, -1
