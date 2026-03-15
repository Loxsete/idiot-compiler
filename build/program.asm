section .data
x dd 0
str_3 db "yes", 0
str_3_len equ $ - str_3
str_5 db "no", 0
str_5_len equ $ - str_5
section .text
default rel
global main
extern printf
extern puts
user_main:
    mov eax, 10
    mov [x], eax
    mov eax, [x]
    cmp eax, 10
    jne .else_0
    lea rdi, [rel str_3]
    call puts
    jmp .end_0
.else_0:
    lea rdi, [rel str_5]
    call puts
.end_0:
    ret
main:
    call user_main
    xor eax, eax
    ret
