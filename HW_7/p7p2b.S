    .data
mystring: .string "Hello World\n\0"
    .text
main:
    movq $1, %rax
    movq $mystring, %rsi
    movq $14, %rdx
    SYSCALL
