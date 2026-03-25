section .data
p dd 0
section .text
default rel
global main
extern malloc
extern free
user_main:
    mov rdi, 64
    call malloc
    mov [p], eax
    mov rdi, [p]
    call free
    ret
main:
    call user_main
    xor eax, eax
    ret
