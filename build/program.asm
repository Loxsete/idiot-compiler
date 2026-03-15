section .data
fmt_int  db "%d",  10, 0
fmt_long db "%ld", 10, 0
a dd 0
b dd 0
result dd 0
n dd 0
x dd 0
y dd 0
str_14 db "x is less than y", 0
str_14_len equ $ - str_14
str_16 db "x is not less than y", 0
str_16_len equ $ - str_16
str_19 db "x is 10", 0
str_19_len equ $ - str_19
str_21 db "x is not 10", 0
str_21_len equ $ - str_21
str_24 db "x and y are different", 0
str_24_len equ $ - str_24
str_26 db "x and y are equal", 0
str_26_len equ $ - str_26
i dd 0
sum dd 0
str_37 db "for loop iteration", 0
str_37_len equ $ - str_37
big dq 0
total dq 0
p dq 0
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
    lea rdi, [rel fmt_int]
    mov esi, eax
    xor eax, eax
    call printf
    ret
countdown:
    mov [n], edi
.while_0:
    mov eax, [n]
    cmp eax, 0
    jle .wend_0
    mov eax, [n]
    lea rdi, [rel fmt_int]
    mov esi, eax
    xor eax, eax
    call printf
    mov eax, [n]
    sub eax, 1
    mov [n], eax
    jmp .while_0
.wend_0:
    ret
user_main:
    mov eax, 10
    mov [x], eax
    mov eax, 20
    mov [y], eax
    mov eax, [x]
    cmp eax, [y]
    jge .else_0
    lea rdi, [rel str_14]
    call puts
    jmp .end_0
.else_0:
    lea rdi, [rel str_16]
    call puts
.end_0:
    mov eax, [x]
    cmp eax, 10
    jne .else_1
    lea rdi, [rel str_19]
    call puts
    jmp .end_1
.else_1:
    lea rdi, [rel str_21]
    call puts
.end_1:
    mov eax, [x]
    cmp eax, [y]
    je  .else_2
    lea rdi, [rel str_24]
    call puts
    jmp .end_2
.else_2:
    lea rdi, [rel str_26]
    call puts
.end_2:
    mov rdi, [x]
    mov rsi, [y]
    call add
    mov eax, 0
    mov [i], eax
    mov eax, 0
    mov [sum], eax
.while_1:
    mov eax, [i]
    cmp eax, 100
    jge .wend_1
    mov eax, [sum]
    add eax, [i]
    mov [sum], eax
    mov eax, [i]
    add eax, 1
    mov [i], eax
    jmp .while_1
.wend_1:
    mov eax, [sum]
    lea rdi, [rel fmt_int]
    mov esi, eax
    xor eax, eax
    call printf
    mov dword [i], 0
.for_0:
    mov eax, [i]
    cmp eax, 5
    jge .fend_0
    lea rdi, [rel str_37]
    call puts
    mov eax, [i]
    add eax, 1
    mov [i], eax
    jmp .for_0
.fend_0:
    mov rax, 1000000
    mov [big], rax
    mov rax, 0
    mov [total], rax
    mov dword [i], 0
.for_1:
    mov eax, [i]
    cmp eax, 1000000
    jge .fend_1
    mov eax, [total]
    add eax, 1
    mov [total], eax
    mov eax, [i]
    add eax, 1
    mov [i], eax
    jmp .for_1
.fend_1:
    mov rax, [total]
    lea rdi, [rel fmt_long]
    mov rsi, rax
    xor eax, eax
    call printf
    lea rax, [x]
    mov [p], rax
    mov eax, [x]
    lea rdi, [rel fmt_int]
    mov esi, eax
    xor eax, eax
    call printf
    mov eax, 99
    mov rcx, [p]
    mov [rcx], eax
    mov eax, [x]
    lea rdi, [rel fmt_int]
    mov esi, eax
    xor eax, eax
    call printf
    mov rdi, 5
    call countdown
    ret
main:
    call user_main
    xor eax, eax
    ret
