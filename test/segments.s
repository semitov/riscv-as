# segments example (using spike and pk)

.data
var1: .word 3 # declare a word with an initial value
array1: .space 40 # declare an array with 40 bytes
hellostr: .asciiz "Hello, world!\n" # null-terminated string
hellostr2: .string "Hello, world!\n" # another alias for string

.text
li a0, 0           # return code
li a7, 93          # exit syscall
ecall
