section .data
x dd 0
str_3 db "yes", 10
str_3_len equ $ - str_3
str_5 db "no", 10
str_5_len equ $ - str_5
section .text
global _start
main:
    mov eax, 10
    mov [x], eax
    mov eax, [x]
    cmp eax, 10
    jne .else_0
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel str_3]
    mov rdx, str_3_len
    syscall
    jmp .end_0
.else_0:
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel str_5]
    mov rdx, str_5_len
    syscall
.end_0:
    ret
_start:
    call main
    mov rax, 60
    xor rdi, rdi
    syscall
