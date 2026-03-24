#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "codegen.h"

static int is_num(const char *s) { return isdigit((unsigned char)s[0]); }

static const char *CALL_REGS[]   = { "rdi", "rsi", "rdx", "rcx", "r8",  "r9"  };
static const char *CALL_REGS32[] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };

static void emit_node(FILE *f, AST *a, int idx)
{
    switch (a->type) {

    case AST_ASSIGN: {
        if (a->deref_assign) {
            if (is_num(a->left_str))
                fprintf(f, "    mov eax, %s\n", a->left_str);
            else
                fprintf(f, "    mov eax, [%s]\n", a->left_str);
            fprintf(f, "    mov rcx, [%s]\n", a->var);
            fprintf(f, "    mov [rcx], eax\n");
            break;
        }

        if (a->is_ref) {
            fprintf(f, "    lea rax, [%s]\n", a->left_str);
            fprintf(f, "    mov [%s], rax\n", a->var);
            break;
        }

        if (a->is_deref) {
            fprintf(f, "    mov rax, [%s]\n", a->left_str);
            fprintf(f, "    mov eax, [rax]\n");
            fprintf(f, "    mov [%s], eax\n", a->var);
            break;
        }

        int is_long = (a->var_type == VAR_LONG);
        const char *acc = is_long ? "rax" : "eax";

        if (is_num(a->left_str))
            fprintf(f, "    mov %s, %s\n", acc, a->left_str);
        else
            fprintf(f, "    mov %s, [%s]\n", acc, a->left_str);

        if (a->op_sign[0]) {
            int rn = is_num(a->right_str);
            if (strcmp(a->op_sign, "+") == 0)
                fprintf(f, rn ? "    add %s, %s\n" : "    add %s, [%s]\n", acc, a->right_str);
            else if (strcmp(a->op_sign, "-") == 0)
                fprintf(f, rn ? "    sub %s, %s\n" : "    sub %s, [%s]\n", acc, a->right_str);
            else if (strcmp(a->op_sign, "*") == 0)
                fprintf(f, rn ? "    imul %s, %s\n" : "    imul %s, [%s]\n", acc, a->right_str);
            else if (strcmp(a->op_sign, "/") == 0) {
                fprintf(f, is_long ? "    cqo\n" : "    cdq\n");
                if (rn) fprintf(f, "    mov rcx, %s\n    idiv rcx\n", a->right_str);
                else    fprintf(f, is_long ? "    idiv qword [%s]\n" : "    idiv dword [%s]\n", a->right_str);
            }
        }
        fprintf(f, "    mov [%s], %s\n", a->var, acc);
        break;
    }

    case AST_ASSIGN_CALL:
    	/* all arguments in reg (System V ABI)
		some this: first argument ---> rdi, second argument ---> rsi ...
    	*/
    	for (int j = 0; j < a->arg_count; j++) {
    	    if (is_num(a->args[j]))
    	        fprintf(f, "    mov %s, %s\n", CALL_REGS[j], a->args[j]);
    	    else
    	        fprintf(f, "    mov %s, [%s]\n", CALL_REGS[j], a->args[j]);
    	}
    	fprintf(f, "    call %s\n", a->left_str);
    	fprintf(f, "    mov [%s], eax\n", a->var);
    	break;

    case AST_ASSIGN_STR:
        break;

    case AST_PRINT:
        if (a->var_type == VAR_LONG) {
            fprintf(f,
                "    mov rax, [%s]\n"
                "    lea rdi, [rel fmt_long]\n"
                "    mov rsi, rax\n"
                "    xor eax, eax\n"
                "    call printf\n",
                a->var);
        } else {
            fprintf(f,
                "    mov eax, [%s]\n"
                "    lea rdi, [rel fmt_int]\n"
                "    mov esi, eax\n"
                "    xor eax, eax\n"
                "    call printf\n",
                a->var);
        }
        break;

    case AST_PRINT_STR:
        if (a->has_newline) {
            fprintf(f,
                "    lea rdi, [rel str_%d]\n"
                "    call puts\n",
                idx);
        } else {
            fprintf(f,
                "    lea rdi, [rel fmt_str]\n"
                "    lea rsi, [rel str_%d]\n"
                "    xor eax, eax\n"
                "    call printf\n",
                idx);
        }
        break;

    case AST_FUNC_CALL:
        for (int j = 0; j < a->arg_count; j++) {
            if (is_num(a->args[j]))
                fprintf(f, "    mov %s, %s\n", CALL_REGS[j], a->args[j]);
            else
                fprintf(f, "    mov %s, [%s]\n", CALL_REGS[j], a->args[j]);
        }
        fprintf(f, "    call %s\n", a->var);
        break;

    case AST_RETURN: {
        if (a->left_str[0] != '\0') {
            int is_long = (a->var_type == VAR_LONG);
            const char *acc = is_long ? "rax" : "eax";
            if (is_num(a->left_str))
                fprintf(f, "    mov %s, %s\n", acc, a->left_str);
            else
                fprintf(f, "    mov %s, [%s]\n", acc, a->left_str);
        }
        fprintf(f, "    ret\n");
        break;
    }

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

    case AST_WHILE:
        fprintf(f, ".while_%d:\n", a->while_id);
        if (is_num(a->cmp_left))
            fprintf(f, "    mov eax, %s\n", a->cmp_left);
        else
            fprintf(f, "    mov eax, [%s]\n", a->cmp_left);
        if (is_num(a->cmp_right))
            fprintf(f, "    cmp eax, %s\n", a->cmp_right);
        else
            fprintf(f, "    cmp eax, [%s]\n", a->cmp_right);
        if      (strcmp(a->op, "==") == 0) fprintf(f, "    jne .wend_%d\n", a->while_id);
        else if (strcmp(a->op, "!=") == 0) fprintf(f, "    je  .wend_%d\n", a->while_id);
        else if (strcmp(a->op, "<")  == 0) fprintf(f, "    jge .wend_%d\n", a->while_id);
        else if (strcmp(a->op, ">")  == 0) fprintf(f, "    jle .wend_%d\n", a->while_id);
        break;

    case AST_WHILE_END:
        fprintf(f, "    jmp .while_%d\n.wend_%d:\n", a->while_id, a->while_id);
        break;

    case AST_FOR:
        if (is_num(a->for_init_val))
            fprintf(f, "    mov dword [%s], %s\n", a->for_init_var, a->for_init_val);
        else
            fprintf(f, "    mov eax, [%s]\n    mov [%s], eax\n", a->for_init_val, a->for_init_var);
        fprintf(f, ".for_%d:\n", a->while_id);
        if (is_num(a->cmp_left))
            fprintf(f, "    mov eax, %s\n", a->cmp_left);
        else
            fprintf(f, "    mov eax, [%s]\n", a->cmp_left);
        if (is_num(a->cmp_right))
            fprintf(f, "    cmp eax, %s\n", a->cmp_right);
        else
            fprintf(f, "    cmp eax, [%s]\n", a->cmp_right);
        if      (strcmp(a->op, "==") == 0) fprintf(f, "    jne .fend_%d\n", a->while_id);
        else if (strcmp(a->op, "!=") == 0) fprintf(f, "    je  .fend_%d\n", a->while_id);
        else if (strcmp(a->op, "<")  == 0) fprintf(f, "    jge .fend_%d\n", a->while_id);
        else if (strcmp(a->op, ">")  == 0) fprintf(f, "    jle .fend_%d\n", a->while_id);
        break;

    case AST_FOR_END:
        if (is_num(a->for_step_val))
            fprintf(f, "    mov eax, [%s]\n    %s eax, %s\n    mov [%s], eax\n",
                a->for_step_var,
                strcmp(a->for_step_op, "+") == 0 ? "add" : "sub",
                a->for_step_val,
                a->for_step_var);
        else
            fprintf(f, "    mov eax, [%s]\n    mov ecx, [%s]\n    %s eax, ecx\n    mov [%s], eax\n",
                a->for_step_var, a->for_step_val,
                strcmp(a->for_step_op, "+") == 0 ? "add" : "sub",
                a->for_step_var);
        fprintf(f, "    jmp .for_%d\n.fend_%d:\n", a->while_id, a->while_id);
        break;

    case AST_FUNC_DEF:
    case AST_FUNC_END:
        break;
    }
}

static void emit_data(FILE *f, AST *nodes, int n)
{
    fprintf(f, "section .data\n");

    int need_fmt_int = 0, need_fmt_str = 0, need_fmt_long = 0;
    for (int i = 0; i < n; i++) {
        if (nodes[i].type == AST_PRINT) {
            if (nodes[i].var_type == VAR_LONG) need_fmt_long = 1;
            else                               need_fmt_int  = 1;
        }
        if (nodes[i].type == AST_PRINT_STR && !nodes[i].has_newline) need_fmt_str = 1;
    }
    if (need_fmt_int)  fprintf(f, "fmt_int  db \"%%d\",  10, 0\n");
    if (need_fmt_long) fprintf(f, "fmt_long db \"%%ld\", 10, 0\n");
    if (need_fmt_str)  fprintf(f, "fmt_str  db \"%%s\",  0\n");

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
                else if (a->var_type == VAR_PTR)  fprintf(f, "%s dq 0\n", a->var);
                else if (a->var_type == VAR_LONG) fprintf(f, "%s dq 0\n", a->var);
                else                              fprintf(f, "%s dd 0\n", a->var);
                strcpy(declared[dc++], a->var);
            }
        }
        else if (a->type == AST_ASSIGN_CALL) {
            int dup = 0;
            for (int j = 0; j < dc; j++)
                if (strcmp(declared[j], a->var) == 0) { dup = 1; break; }
            if (!dup) {
                fprintf(f, "%s dd 0\n", a->var);
                strcpy(declared[dc++], a->var);
            }
        }
        else if (a->type == AST_FOR) {
            int dup = 0;
            for (int j = 0; j < dc; j++)
                if (strcmp(declared[j], a->for_init_var) == 0) { dup = 1; break; }
            if (!dup) {
                fprintf(f, "%s dd 0\n", a->for_init_var);
                strcpy(declared[dc++], a->for_init_var);
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
                    fprintf(f, "%s dd 0\n", a->args[k]);
                    strcpy(declared[dc++], a->args[k]);
                }
            }
        }
        else if (a->type == AST_PRINT_STR) {
            fprintf(f, "str_%d db \"%s\", 0\n", i, a->var);
            fprintf(f, "str_%d_len equ $ - str_%d\n", i, i);
        }
    }
}

static void emit_text(FILE *f, AST *nodes, int n)
{
    fprintf(f,
            "section .text\n"
            "default rel\n"
            "global main\n"
            "extern printf\n"
            "extern puts\n");

    int in_func = 0;

    for (int i = 0; i < n; i++) {
        AST *a = &nodes[i];

        if (a->type == AST_FUNC_DEF) {
            if (strcmp(a->var, "main") == 0)
                fprintf(f, "user_main:\n");
            else
                fprintf(f, "%s:\n", a->var);
            for (int j = 0; j < a->arg_count; j++)
                fprintf(f, "    mov [%s], %s\n", a->args[j], CALL_REGS32[j]);
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
        "main:\n"
        "    call user_main\n"
        "    xor eax, eax\n"
        "    ret\n");
}

void codegen(FILE *out, AST *nodes, int node_count)
{
    emit_data(out, nodes, node_count);
    emit_text(out, nodes, node_count);
}
