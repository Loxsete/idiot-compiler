#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"


static int sym(const Token *t, const char *s) {

    return strcmp(t->text, s) == 0;
}


AST parse(Token *tokens, int count) {
    AST a;
    memset(&a, 0, sizeof(a));
    a.if_id    = -1;
    a.right    = -1;

    if (sym(&tokens[0], "fn")) {
        a.type = AST_FUNC_DEF;
        strcpy(a.var, tokens[1].text);
        int j = 3;
        while (j < count && !sym(&tokens[j], ")")) {
            if (tokens[j].type == TOK_IDENT)
                strcpy(a.args[a.arg_count++], tokens[j].text);
            j++;
        }
        return a;
    }

    if (sym(&tokens[0], "if")) {
        a.type = AST_IF;
        strcpy(a.cmp_left,  tokens[1].text);
        strcpy(a.op,        tokens[2].text);
        strcpy(a.cmp_right, tokens[3].text);
        return a;
    }

    if (sym(&tokens[0], "}") && count >= 2 && sym(&tokens[1], "else")) {
        a.type = AST_ELSE;
        return a;
    }

    if (sym(&tokens[0], "}")) {
        a.type = AST_FUNC_END;
        return a;
    }

    if (sym(&tokens[0], "print")) {
        if (tokens[2].type == TOK_STRING) {
            a.type = AST_PRINT_STR;
            char *text = tokens[2].text;
            int   len  = strlen(text);
            if (len >= 2 && text[len-2] == '\\' && text[len-1] == 'n') {
                text[len-2]  = '\0';
                a.has_newline = 1;
            }
            strcpy(a.var, text);
            return a;
        }
        a.type = AST_PRINT;
        strcpy(a.var, tokens[2].text);
        return a;
    }

    if (tokens[0].type == TOK_IDENT && sym(&tokens[1], "(")) {
        a.type = AST_FUNC_CALL;
        strcpy(a.var, tokens[0].text);
        int j = 2;
        while (j < count && !sym(&tokens[j], ")")) {
            if (tokens[j].type == TOK_NUMBER || tokens[j].type == TOK_IDENT)
                strcpy(a.args[a.arg_count++], tokens[j].text);
            j++;
        }
        return a;
    }

    
    if (tokens[0].type == TOK_IDENT &&
        (strcmp(tokens[0].text, "int")  == 0 ||
         strcmp(tokens[0].text, "char") == 0 ||
         strcmp(tokens[0].text, "bool") == 0))
    {
        if (count < 4 || !sym(&tokens[2], "=")) {
            fprintf(stderr, "parser: bad declaration near '%s'\n", tokens[1].text);
            exit(1);
        }
    
        if      (strcmp(tokens[0].text, "int")  == 0) a.var_type = VAR_INT;
        else if (strcmp(tokens[0].text, "char") == 0) a.var_type = VAR_CHAR;
        else if (strcmp(tokens[0].text, "bool") == 0) a.var_type = VAR_BOOL;
    
        strcpy(a.var, tokens[1].text);
    
        if (a.var_type == VAR_CHAR && tokens[3].type == TOK_STRING) {
            a.type = AST_ASSIGN_STR;
            char *text = tokens[3].text;
            int   len  = strlen(text);
            if (len >= 2 && text[len-2] == '\\' && text[len-1] == 'n') {
                text[len-2]  = '\0';
                a.has_newline = 1;
            }
            strcpy(a.left_str, text);
        } else {
            a.type = AST_ASSIGN;
            strcpy(a.left_str, tokens[3].text);
            a.left = tokens[3].value;
            if (count > 5) {
                strcpy(a.op_sign,   tokens[4].text);
                strcpy(a.right_str, tokens[5].text);
                a.right = tokens[5].value;
            }
        }
        return a;
    }

    if (count < 3 || !sym(&tokens[1], "=")) {
        fprintf(stderr, "parser: syntax error near '%s'\n", tokens[0].text);
        exit(1);
    }

    a.type = AST_ASSIGN;
    strcpy(a.var,      tokens[0].text);
    strcpy(a.left_str, tokens[2].text);
    a.left = tokens[2].value;

    if (count > 3) {
        strcpy(a.op_sign,   tokens[3].text);
        strcpy(a.right_str, tokens[4].text);
        a.right = tokens[4].value;
    }

    return a;
}

char *read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror(filename); exit(1); }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc(size + 1);
    if (!buf) { perror("malloc"); exit(1); }

    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}
