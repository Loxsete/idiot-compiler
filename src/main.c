#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "ast.h"

#define MAX_NODES 256

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file.ic> [output]\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_asm = argc >= 3 ? argv[2] : "build/program.asm";

    char *source = read_file(input_path);

    AST nodes[MAX_NODES];
    int node_count = 0;

    typedef struct { char name[256]; VarType type; int str_node_idx; } VarInfo;
    VarInfo var_table[64];
    int     var_count = 0;

    int if_stack[32];
    int if_top     = 0;
    int if_counter = 0;
    int brace_depth = 0;

    char *line = strtok(source, "\n");
    while (line) {
        Token tokens[64];
        int   count = tokenize(line, tokens);
        if (count > 0) {
            AST node = parse(tokens, count);

            if (node.type == AST_FUNC_DEF) {
                brace_depth++;
            }
            else if (node.type == AST_ASSIGN || node.type == AST_ASSIGN_STR) {
                var_table[var_count].type         = node.var_type;
                var_table[var_count].str_node_idx = node_count;
                strcpy(var_table[var_count].name, node.var);
                var_count++;
            }
            else if (node.type == AST_PRINT) {
                for (int v = 0; v < var_count; v++) {
                    if (strcmp(var_table[v].name, node.var) == 0) {
                        if (var_table[v].type == VAR_CHAR) {
                            node.type  = AST_PRINT_STR;
                            node.if_id = var_table[v].str_node_idx;
                        }
                        break;
                    }
                }
            }
            else if (node.type == AST_IF) {
                node.if_id = if_counter;
                if_stack[if_top++] = if_counter++;
                brace_depth++;
            }
            else if (node.type == AST_ELSE) {
                node.if_id = if_stack[if_top - 1];
            }
            else if (node.type == AST_FUNC_END) {
                brace_depth--;
                if (brace_depth > 0) {
                    node.type  = AST_IF_END;
                    node.if_id = if_stack[--if_top];
                }
            }

            if (node_count >= MAX_NODES) {
                fprintf(stderr, "error: too many nodes (max %d)\n", MAX_NODES);
                exit(1);
            }
            nodes[node_count++] = node;
        }
        line = strtok(NULL, "\n");
    }

    FILE *out = fopen(output_asm, "w");
    if (!out) { perror(output_asm); return 1; }
    codegen(out, nodes, node_count);
    fclose(out);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "nasm -felf64 %s -o build/program.o", output_asm);
    if (system(cmd) != 0) { fprintf(stderr, "nasm failed\n"); return 1; }
    if (system("ld build/program.o -o program") != 0) {
        fprintf(stderr, "ld failed\n");
        return 1;
    }

    printf("done → build/program\n");
    free(source);
    return 0;
}
