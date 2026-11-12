.data
x: .word 32

.bss
foo: .zero 10
exceed: .zero 2048

.text
la a1, foo
