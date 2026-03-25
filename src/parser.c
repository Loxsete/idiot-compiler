#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static int sym(const Token *t, const char *s) {
    return strcmp(t->text, s) == 0;
}

static void parse_error(const char *msg, const char *near) {
    fprintf(stderr, "parser: %s near '%s'\n", msg, near);
    exit(1);
}

AST parse(Token *tokens, int count) {
    AST a;
    memset(&a, 0, sizeof(a));
    a.if_id = -1;
    a.right = -1;

    if (sym(&tokens[0], "fn")) {
        a.type = AST_FUNC_DEF;
        if (count < 2 || tokens[1].type != TOK_IDENT)
            parse_error("expected function name after 'fn'", tokens[0].text);
        strcpy(a.var, tokens[1].text);
        int j = 3;
        while (j < count && !sym(&tokens[j], ")") && !sym(&tokens[j], "{")) {
            if (tokens[j].type == TOK_IDENT)
                strcpy(a.args[a.arg_count++], tokens[j].text);
            j++;
        }
        return a;
    }

    if (sym(&tokens[0], "if")) {
        if (count < 4)
            parse_error("incomplete if condition", tokens[0].text);
        a.type = AST_IF;
        strcpy(a.cmp_left,  tokens[1].text);
        strcpy(a.op,        tokens[2].text);
        strcpy(a.cmp_right, tokens[3].text);
        if (count > 4 && !(count == 5 && sym(&tokens[4], "{")))
            parse_error("unexpected tokens after if condition", tokens[4].text);
        return a;
    }

    if (sym(&tokens[0], "while")) {
        if (count < 4)
            parse_error("incomplete while condition", tokens[0].text);
        a.type = AST_WHILE;
        strcpy(a.cmp_left,  tokens[1].text);
        strcpy(a.op,        tokens[2].text);
        strcpy(a.cmp_right, tokens[3].text);
        return a;
    }

    if (sym(&tokens[0], "for")) {
        if (count < 14)
            parse_error("incomplete for loop", tokens[0].text);
        a.type = AST_FOR;
        strcpy(a.for_init_var, tokens[2].text);
        strcpy(a.for_init_val, tokens[4].text);
        strcpy(a.cmp_left,     tokens[6].text);
        strcpy(a.op,           tokens[7].text);
        strcpy(a.cmp_right,    tokens[8].text);
        strcpy(a.for_step_var, tokens[10].text);
        strcpy(a.for_step_op,  tokens[13].text);
        strcpy(a.for_step_val, tokens[14].text);
       /* This is for debug, i think now its not need 
       for (int _d = 0; _d < count; _d++)
            fprintf(stderr, "tok[%d] = '%s'\n", _d, tokens[_d].text);
       */
        return a;
    }

    if (sym(&tokens[0], "}") && count >= 2 && sym(&tokens[1], "else")) {
        if (count > 2 && !(count == 3 && sym(&tokens[2], "{")))
            parse_error("unexpected tokens after '} else'", tokens[2].text);
        a.type = AST_ELSE;
        return a;
    }

    if (sym(&tokens[0], "}")) {
        if (count > 1)
            parse_error("unexpected tokens after '}'", tokens[1].text);
        a.type = AST_FUNC_END;
        return a;
    }

    if (sym(&tokens[0], "print")) {
        if (count < 3 || !sym(&tokens[1], "("))
            parse_error("expected '(' after 'print'", tokens[0].text);
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
        if (tokens[2].type != TOK_IDENT && tokens[2].type != TOK_NUMBER)
            parse_error("expected variable or string in print()", tokens[2].text);
        a.type = AST_PRINT;
        strcpy(a.var, tokens[2].text);
        return a;
    }
	// yeah return (james hatfield referenese yeah)
    if (sym(&tokens[0], "return")) {
	    a.type = AST_RETURN;
	    if (count >= 2) {
		    strcpy(a.left_str, tokens[1].text);
		    a.var_type = VAR_INT;
	    } else {
		a.left_str[0] = '\0';
	    }
	    return a;
    }

    if (sym(&tokens[0], "extern")) {
	if (count < 2 || tokens[1].type != TOK_IDENT)
            parse_error("what is this after extern?", tokens[0].text);
	a.type = AST_EXTERN;
	strcpy(a.var, tokens[1].text);
	return a;
    }

    if (tokens[0].type == TOK_IDENT && count >= 2 && sym(&tokens[1], "(")) {
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

    if (sym(&tokens[0], "*")) {
        if (count < 4 || tokens[1].type != TOK_IDENT || !sym(&tokens[2], "="))
            parse_error("invalid deref assignment", tokens[0].text);
        a.type         = AST_ASSIGN;
        a.deref_assign = 1;
        strcpy(a.var,      tokens[1].text);
        strcpy(a.left_str, tokens[3].text);
        return a;
    }

    if (tokens[0].type == TOK_IDENT && strcmp(tokens[0].text, "ptr") == 0
        && count >= 2 && sym(&tokens[1], "*"))
    {
        if (count < 5)
            parse_error("incomplete ptr declaration", tokens[0].text);
        a.type     = AST_ASSIGN;
        a.var_type = VAR_PTR;
        strcpy(a.var, tokens[2].text);
        if (count >= 6 && sym(&tokens[4], "&")) {
            a.is_ref = 1;
            strcpy(a.left_str, tokens[5].text);
        } else {
            strcpy(a.left_str, tokens[4].text);
        }
        return a;
    }

    if (tokens[0].type == TOK_IDENT &&
        (strcmp(tokens[0].text, "int")  == 0 ||
         strcmp(tokens[0].text, "char") == 0 ||
         strcmp(tokens[0].text, "bool") == 0 ||
         strcmp(tokens[0].text, "long") == 0))
    {
        if (count < 4 || !sym(&tokens[2], "="))
            parse_error("bad declaration", tokens[1].text);

        if      (strcmp(tokens[0].text, "int")  == 0) a.var_type = VAR_INT;
        else if (strcmp(tokens[0].text, "char") == 0) a.var_type = VAR_CHAR;
        else if (strcmp(tokens[0].text, "bool") == 0) a.var_type = VAR_BOOL;
        else if (strcmp(tokens[0].text, "long") == 0) a.var_type = VAR_LONG;

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
        } else if (count >= 5 && sym(&tokens[3], "*") && tokens[4].type == TOK_IDENT) {
            a.type     = AST_ASSIGN;
            a.is_deref = 1;
            strcpy(a.left_str, tokens[4].text);
        } else if (tokens[3].type == TOK_IDENT && count >= 5 && sym(&tokens[4], "(")) {
            // this must work (i think) int x = func(args)
            a.type = AST_ASSIGN_CALL;
            strcpy(a.left_str, tokens[3].text);  // name func
            int j = 5;
            while (j < count && !sym(&tokens[j], ")")) {
                if (tokens[j].type == TOK_NUMBER || tokens[j].type == TOK_IDENT)
                    strcpy(a.args[a.arg_count++], tokens[j].text);
                j++;
        	}
        } else {
            if (tokens[3].type != TOK_IDENT && tokens[3].type != TOK_NUMBER)
                parse_error("expected value in declaration", tokens[3].text);
            a.type = AST_ASSIGN;
            strcpy(a.left_str, tokens[3].text);
            a.left = tokens[3].value;
            if (count >= 6) {
                const char *op = tokens[4].text;
                if (strcmp(op,"+")!=0 && strcmp(op,"-")!=0 &&
                    strcmp(op,"*")!=0 && strcmp(op,"/")!=0)
                    parse_error("unknown operator", op);
                strcpy(a.op_sign,   tokens[4].text);
                strcpy(a.right_str, tokens[5].text);
                a.right = tokens[5].value;
            }
        }
        return a;
    }

    if (count < 3 || !sym(&tokens[1], "="))
        parse_error("syntax error", tokens[0].text);

    a.type = AST_ASSIGN;
    strcpy(a.var,      tokens[0].text);
    strcpy(a.left_str, tokens[2].text);
    a.left = tokens[2].value;

    if (count >= 5) {
        const char *op = tokens[3].text;
        if (strcmp(op,"+")!=0 && strcmp(op,"-")!=0 &&
            strcmp(op,"*")!=0 && strcmp(op,"/")!=0)
            parse_error("unknown operator", op);
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
