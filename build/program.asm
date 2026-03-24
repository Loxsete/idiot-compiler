section .data
fmt_int  db "%d",  10, 0
a dd 0
b dd 0
result dd 0
x dd 0
section .text
default rel
global main
extern printf
extern puts
add:
    mov [a], edi
    mov [b], esi
    mov eax, [a]
    add eax, [b]
    mov [result], eax
    mov eax, [result]
    ret
    ret
user_main:
    mov rdi, 3
    mov rsi, 4
    call add
    mov [x], eax
    mov eax, [x]
    lea rdi, [rel fmt_int]
    mov esi, eax
    xor eax, eax
    call printf
    ret
main:
    call user_main
    xor eax, eax
    ret
