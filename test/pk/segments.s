# segments example (using spike and pk)

.data
msg: .asciiz "SEMITO-V\n"

.text
li a0, 0 # return code
li a7, 93 # sys_exit
ecall
