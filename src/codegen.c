#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "codegen.h"

static int is_num(const char *s) { return isdigit((unsigned char)s[0]); }

static const char *CALL_REGS[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

static void emit_node(FILE *f, AST *a, int idx)
{
    switch (a->type) {

    case AST_ASSIGN:
        if (is_num(a->left_str))
            fprintf(f, "    mov eax, %s\n", a->left_str);
        else
            fprintf(f, "    mov eax, [%s]\n", a->left_str);

        if (a->op_sign[0]) {
            int rn = is_num(a->right_str);
            if      (strcmp(a->op_sign, "+") == 0)
                fprintf(f, rn ? "    add eax, %s\n"
                              : "    add eax, [%s]\n", a->right_str);
            else if (strcmp(a->op_sign, "-") == 0)
                fprintf(f, rn ? "    sub eax, %s\n"
                              : "    sub eax, [%s]\n", a->right_str);
            else if (strcmp(a->op_sign, "*") == 0)
                fprintf(f, rn ? "    imul eax, %s\n"
                              : "    imul eax, [%s]\n", a->right_str);
            else if (strcmp(a->op_sign, "/") == 0) {
                fprintf(f, "    cdq\n");
                if (rn) { fprintf(f, "    mov ecx, %s\n    idiv ecx\n", a->right_str); }
                else      fprintf(f, "    idiv dword [%s]\n", a->right_str);
            }
        }
        fprintf(f, "    mov [%s], eax\n", a->var);
        break;

    case AST_ASSIGN_STR:
        break;

    case AST_PRINT:
        fprintf(f,
            "    mov eax, [%s]\n"
            "    lea rsi, [rel num_buf+19]\n"
            "    mov byte [rsi], 10\n"
            "    dec rsi\n"
            "    mov ecx, 10\n"
            ".digit_loop_%d:\n"
            "    xor edx, edx\n"
            "    div ecx\n"
            "    add dl, '0'\n"
            "    mov [rsi], dl\n"
            "    dec rsi\n"
            "    test eax, eax\n"
            "    jnz .digit_loop_%d\n"
            "    inc rsi\n"
            "    lea rdx, [rel num_buf+20]\n"
            "    sub rdx, rsi\n"
            "    mov rax, 1\n"
            "    mov rdi, 1\n"
            "    syscall\n",
            a->var, idx, idx);
        break;

    case AST_PRINT_STR:
        fprintf(f,
            "    mov rax, 1\n"
            "    mov rdi, 1\n"
            "    lea rsi, [rel str_%d]\n"
            "    mov rdx, str_%d_len\n"
            "    syscall\n",
            a->if_id, a->if_id);
        break;

    case AST_FUNC_CALL:
        for (int j = 0; j < a->arg_count; j++)
            fprintf(f, "    mov %s, %s\n", CALL_REGS[j], a->args[j]);
        fprintf(f, "    call %s\n", a->var);
        break;

    case AST_IF:
        if (is_num(a->cmp_left))
            fprintf(f, "    mov eax, %s\n", a->cmp_left);
        else
            fprintf(f, "    mov eax, [%s]\n", a->cmp_left);
        if (is_num(a->cmp_right))
            fprintf(f, "    cmp eax, %s\n", a->cmp_right);
        else
            fprintf(f, "    cmp eax, [%s]\n", a->cmp_right);
        if      (strcmp(a->op, "==") == 0) fprintf(f, "    jne .else_%d\n", a->if_id);
        else if (strcmp(a->op, "!=") == 0) fprintf(f, "    je  .else_%d\n", a->if_id);
        else if (strcmp(a->op, "<")  == 0) fprintf(f, "    jge .else_%d\n", a->if_id);
        else if (strcmp(a->op, ">")  == 0) fprintf(f, "    jle .else_%d\n", a->if_id);
        break;

    case AST_ELSE:
        fprintf(f, "    jmp .end_%d\n.else_%d:\n", a->if_id, a->if_id);
        break;

    case AST_IF_END:
        fprintf(f, ".end_%d:\n", a->if_id);
        break;

    case AST_FUNC_DEF:
    case AST_FUNC_END:
        break;
    }
}

static void emit_data(FILE *f, AST *nodes, int n)
{
    fprintf(f, "section .data\n");

    for (int i = 0; i < n; i++)
        if (nodes[i].type == AST_PRINT) { fprintf(f, "num_buf times 20 db 0\n"); break; }

    char declared[64][256];
    int  dc = 0;

    for (int i = 0; i < n; i++) {
        AST *a = &nodes[i];

        if (a->type == AST_ASSIGN) {
            int dup = 0;
            for (int j = 0; j < dc; j++)
                if (strcmp(declared[j], a->var) == 0) { dup = 1; break; }
            if (!dup) {
                if      (a->var_type == VAR_BOOL) fprintf(f, "%s db 0\n", a->var);
                else if (a->var_type == VAR_INT)  fprintf(f, "%s dd 0\n", a->var);
                else                              fprintf(f, "%s dd 0\n", a->var);
                strcpy(declared[dc++], a->var);
            }
        }
        else if (a->type == AST_ASSIGN_STR) {
            if (a->has_newline)
                fprintf(f, "str_%d db \"%s\", 10\n", i, a->left_str);
            else
                fprintf(f, "str_%d db \"%s\"\n", i, a->left_str);
            fprintf(f, "str_%d_len equ $ - str_%d\n", i, i);
        }
        else if (a->type == AST_FUNC_DEF) {
            for (int k = 0; k < a->arg_count; k++) {
                int dup = 0;
                for (int j = 0; j < dc; j++)
                    if (strcmp(declared[j], a->args[k]) == 0) { dup = 1; break; }
                if (!dup) {
                    fprintf(f, "%s dq 0\n", a->args[k]);
                    strcpy(declared[dc++], a->args[k]);
                }
            }
        }
        else if (a->type == AST_PRINT_STR) {
            if (a->has_newline)
                fprintf(f, "str_%d db \"%s\", 10\n", i, a->var);
            else
                fprintf(f, "str_%d db \"%s\"\n", i, a->var);
            fprintf(f, "str_%d_len equ $ - str_%d\n", i, i);
        }
    }
}

static void emit_text(FILE *f, AST *nodes, int n)
{
    fprintf(f, "section .text\nglobal _start\n");

    int in_func = 0;

    for (int i = 0; i < n; i++) {
        AST *a = &nodes[i];

        if (a->type == AST_FUNC_DEF) {
            fprintf(f, "%s:\n", a->var);
            for (int j = 0; j < a->arg_count; j++)
                fprintf(f, "    mov [%s], %s\n", a->args[j], CALL_REGS[j]);
            in_func = 1;
        }
        else if (a->type == AST_FUNC_END && in_func) {
            fprintf(f, "    ret\n");
            in_func = 0;
        }
        else if (in_func) {
            emit_node(f, a, i);
        }
    }

    fprintf(f,
        "_start:\n"
        "    call main\n"
        "    mov rax, 60\n"
        "    xor rdi, rdi\n"
        "    syscall\n");
}

void codegen(FILE *out, AST *nodes, int node_count)
{
    emit_data(out, nodes, node_count);
    emit_text(out, nodes, node_count);
}
