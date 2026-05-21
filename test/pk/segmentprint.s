# segments print example (using spike and pk)

.data
msg: .asciiz "SEMITO-V\n"

.text
li a7, 64 # sys_write
li a0, 1 # stdout
la a1, msg # load msg address into a1
li a2, 10 # length
ecall

li a0, 0 # return code
li a7, 93 # sys_exit
ecall
