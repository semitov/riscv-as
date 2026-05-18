# Write "SEMITO-V\n" on stdout

# reserve stack size
addi sp, sp, -16

li t0, 83          # 'S'
sb t0, 0(sp)
li t0, 69          # 'E'
sb t0, 1(sp)
li t0, 77          # 'M'
sb t0, 2(sp)
li t0, 73          # 'I'
sb t0, 3(sp)
li t0, 84          # 'T'
sb t0, 4(sp)
li t0, 79          # 'O'
sb t0, 5(sp)
li t0, 45          # '-'
sb t0, 6(sp)
li t0, 86          # 'V'
sb t0, 7(sp)
li t0, 10          # '\n'
sb t0, 8(sp)

li a0, 1           # fd = stdout
addi a1, sp, 0     # buf = sp
li a2, 9           # count = 9
li a7, 64          # write syscall
ecall

li a0, 0           # return code
li a7, 93          # exit syscall
ecall
