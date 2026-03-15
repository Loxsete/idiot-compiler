section .data
num_buf times 20 db 0
a dd 0
b dd 0
x dd 0
section .text
global _start
main:
    mov eax, 2
    mov [a], eax
    mov eax, 3
    mov [b], eax
    mov eax, [b]
    add eax, [a]
    mov [x], eax
    mov eax, [x]
    lea rsi, [rel num_buf+19]
    mov byte [rsi], 10
    dec rsi
    mov ecx, 10
.digit_loop_4:
    xor edx, edx
    div ecx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test eax, eax
    jnz .digit_loop_4
    inc rsi
    lea rdx, [rel num_buf+20]
    sub rdx, rsi
    mov rax, 1
    mov rdi, 1
    syscall
    mov eax, [x]
    cmp eax, 5
    jne .else_0
    mov eax, [5]
    lea rsi, [rel num_buf+19]
    mov byte [rsi], 10
    dec rsi
    mov ecx, 10
.digit_loop_6:
    xor edx, edx
    div ecx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test eax, eax
    jnz .digit_loop_6
    inc rsi
    lea rdx, [rel num_buf+20]
    sub rdx, rsi
    mov rax, 1
    mov rdi, 1
    syscall
.end_0:
    ret
_start:
    call main
    mov rax, 60
    xor rdi, rdi
    syscall
