#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "ast.h"

#define MAX_NODES 256

typedef struct {
    const char *input;
    const char *output;
    int verbose;
    int debug;
} Options;

// i idk, but i think its not need on another code 
static int is_num_str(const char *s) {
    return isdigit((unsigned char)s[0]);
}

static void usage(const char *prog)
{
    fprintf(stderr,
        "usage: %s <file.ic> [options]\n"
        "  -o <file>   output binary (default: a)\n"
        "  -v          verbose output\n"
        "  --debug     keep .asm and .o after build\n",
        prog);
    exit(1);
}

static Options parse_args(int argc, char *argv[])
{
    Options o = { NULL, "a", 0, 0 };
    if (argc < 2) usage(argv[0]);
    o.input = argv[1];
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) { fprintf(stderr, "error: -o requires argument\n"); exit(1); }
            o.output = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0) {
            o.verbose = 1;
        } else if (strcmp(argv[i], "--debug") == 0) {
            o.debug = 1;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            usage(argv[0]);
        }
    }
    return o;
}

static void die(const char *file, int line, const char *msg)
{
    fprintf(stderr, "%s:%d: error: %s\n", file, line, msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    Options opts = parse_args(argc, argv);

    char asm_path[512];
    char obj_path[512];
    snprintf(asm_path, sizeof(asm_path), "build/program.asm");
    snprintf(obj_path, sizeof(obj_path), "build/program.o");

    if (opts.verbose)
        printf(":: reading %s\n", opts.input);

    char *source = read_file(opts.input);

    AST nodes[MAX_NODES];
    int node_count = 0;

    typedef struct { char name[256]; VarType type; int str_node_idx; } VarInfo;
    VarInfo var_table[64];
    int     var_count = 0;

    int if_stack[32];
    int if_top      = 0;
    int if_counter  = 0;

    int while_stack[32];
    int while_top     = 0;
    int while_counter = 0;

    int for_stack[32];
    int for_top     = 0;
    int for_counter = 0;

    int brace_depth = 0;
    int lineno      = 0;

    char *line = strtok(source, "\n");
    while (line) {
        lineno++;
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
                        } else {
                            node.var_type = var_table[v].type;
                        }
                        break;
                    }
                }
            }

            else if (node.type == AST_RETURN) {
                if (node.left_str[0] && !is_num_str(node.left_str)) {
                    for (int v = 0; v < var_count; v++) {
                        if (strcmp(var_table[v].name, node.left_str) == 0) {
                            node.var_type = var_table[v].type;
                            break;
                        }
                    }
                }
            }

            else if (node.type == AST_IF) {
                node.if_id = if_counter;
                if_stack[if_top++] = if_counter++;
                brace_depth++;
            }
            else if (node.type == AST_ELSE) {
                if (if_top == 0) die(opts.input, lineno, "unexpected 'else'");
                node.if_id = if_stack[if_top - 1];
            }
            else if (node.type == AST_WHILE) {
                node.while_id = while_counter;
                while_stack[while_top++] = while_counter++;
                brace_depth++;
            }
            else if (node.type == AST_FOR) {
                node.while_id = for_counter;
                for_stack[for_top++] = for_counter++;
                brace_depth++;
            }
            else if (node.type == AST_FUNC_END) {
                if (brace_depth == 0) die(opts.input, lineno, "unexpected '}'");
                brace_depth--;
                if (brace_depth > 0) {
                    int last_while = while_top > 0 ? while_stack[while_top-1] : -1;
                    int last_if    = if_top    > 0 ? if_stack[if_top-1]       : -1;
                    int last_for   = for_top   > 0 ? for_stack[for_top-1]     : -1;

                    if (for_top > 0 && last_for >= last_while && last_for >= last_if) {
                        node.type     = AST_FOR_END;
                        node.while_id = for_stack[--for_top];
                        for (int fi = 0; fi < node_count; fi++) {
                            if (nodes[fi].type == AST_FOR && nodes[fi].while_id == node.while_id) {
                                strcpy(node.for_step_var, nodes[fi].for_step_var);
                                strcpy(node.for_step_op,  nodes[fi].for_step_op);
                                strcpy(node.for_step_val, nodes[fi].for_step_val);
                                break;
                            }
                        }
                    } else if (while_top > 0 && last_while >= last_if) {
                        node.type     = AST_WHILE_END;
                        node.while_id = while_stack[--while_top];
                    } else {
                        node.type  = AST_IF_END;
                        node.if_id = if_stack[--if_top];
                    }
                }
            }

            if (node_count >= MAX_NODES)
                die(opts.input, lineno, "too many statements (max 256)");

            nodes[node_count++] = node;
        }
        line = strtok(NULL, "\n");
    }

    if (brace_depth != 0)
        die(opts.input, lineno, "unclosed '{'");

    if (opts.verbose)
        printf(":: parsed %d nodes\n", node_count);

    FILE *out = fopen(asm_path, "w");
    if (!out) { perror(asm_path); return 1; }
    codegen(out, nodes, node_count);
    fclose(out);

    if (opts.verbose)
        printf(":: assembling %s\n", asm_path);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "nasm -felf64 %s -o %s 2>&1", asm_path, obj_path);
    if (system(cmd) != 0) {
        fprintf(stderr, "%s: error: assembler failed\n", opts.input);
        return 1;
    }

    if (opts.verbose)
        printf(":: linking %s\n", opts.output);

    snprintf(cmd, sizeof(cmd), "gcc -no-pie %s -o %s 2>&1", obj_path, opts.output);
    if (system(cmd) != 0) {
        fprintf(stderr, "%s: error: linker failed\n", opts.input);
        return 1;
    }

    if (!opts.debug) {
        remove(obj_path);
        if (opts.verbose)
            printf(":: cleaned up intermediate files\n");
    } else {
        printf(":: debug: keeping %s %s\n", asm_path, obj_path);
    }

    printf(":: done -> %s\n", opts.output);
    free(source);
    return 0;
}
