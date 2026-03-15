section .data
num_buf times 20 db 0
x dd 0
p dq 0
y dd 0
section .text
global _start
main:
    mov eax, 42
    mov [x], eax
    lea rax, [x]
    mov [p], rax
    mov rax, [p]
    mov eax, [rax]
    mov [y], eax
    mov eax, [y]
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
    mov eax, 99
    mov rcx, [p]
    mov [rcx], eax
    mov eax, [x]
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
    ret
_start:
    call main
    mov rax, 60
    xor rdi, rdi
    syscall
