section .data
num_buf times 20 db 0
str_1 db "Hello, world?", 10
str_1_len equ $ - str_1
str_2 db "Hello, compiler?", 10
str_2_len equ $ - str_2
compiler_stupid dd 0
str_8 db "Compiler is stupid"
str_8_len equ $ - str_8
str_10 db "Compiler is not stupid"
str_10_len equ $ - str_10
x dd 0
section .text
global _start
hello:
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel str_1]
    mov rdx, str_1_len
    syscall
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel str_2]
    mov rdx, str_2_len
    syscall
    ret
main:
    call hello
    mov eax, 1
    mov [compiler_stupid], eax
    mov eax, [compiler_stupid]
    cmp eax, 1
    jne .else_0
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel str_8]
    mov rdx, str_8_len
    syscall
    jmp .end_0
.else_0:
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel str_10]
    mov rdx, str_10_len
    syscall
.end_0:
    mov eax, 20
    add eax, 10
    mov [x], eax
    mov eax, [x]
    lea rsi, [rel num_buf+19]
    mov byte [rsi], 10
    dec rsi
    mov ecx, 10
.digit_loop_13:
    xor edx, edx
    div ecx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test eax, eax
    jnz .digit_loop_13
    inc rsi
    lea rdx, [rel num_buf+20]
    sub rdx, rsi
    mov rax, 1
    mov rdi, 1
    syscall
    mov eax, 20
    imul eax, 2
    mov [x], eax
    mov eax, [x]
    lea rsi, [rel num_buf+19]
    mov byte [rsi], 10
    dec rsi
    mov ecx, 10
.digit_loop_15:
    xor edx, edx
    div ecx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test eax, eax
    jnz .digit_loop_15
    inc rsi
    lea rdx, [rel num_buf+20]
    sub rdx, rsi
    mov rax, 1
    mov rdi, 1
    syscall
    mov eax, 20
    cdq
    mov ecx, 2
    idiv ecx
    mov [x], eax
    mov eax, [x]
    lea rsi, [rel num_buf+19]
    mov byte [rsi], 10
    dec rsi
    mov ecx, 10
.digit_loop_17:
    xor edx, edx
    div ecx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test eax, eax
    jnz .digit_loop_17
    inc rsi
    lea rdx, [rel num_buf+20]
    sub rdx, rsi
    mov rax, 1
    mov rdi, 1
    syscall
    ret
_start:
    call main
    mov rax, 60
    xor rdi, rdi
    syscall
