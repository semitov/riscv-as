sb  t1, 0(t0)

sh  t2, 2(t0)

sw  t3, 4(t0)

addi t4, t0, 16
sw   t3, -4(t4)


sh gp, 2047(sp)
sw t4, -8(a5)
sw t4, -2048(a5)
sb t5, sp
