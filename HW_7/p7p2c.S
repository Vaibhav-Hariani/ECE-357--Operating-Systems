    .data
mystring: .string "Hello World\n\0"
    .text
main:
    movq $1, %rax
    movq $mystring, %rsi
    movq $13, %rdx
    SYSCALL
    movq %rax, %rdi
    movq $60, %rax
    SYSCALL