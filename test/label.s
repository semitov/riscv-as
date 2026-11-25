.data
foo: .word 4

.text
la a1, foo
loop:
  addi a0, a0, -1
  bne a0, zero, loop
