#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
 
#include "lexer.h"

int tokenize(const char *code, Token *tokens) {
	int count = 0;
	const char *p = code;

	while (*p) {
		if (isspace(*p)) { p++; continue; }

		if (isdigit(*p)) {
			int val = 0;
			while (isdigit(*p)) { val = val * 10 + (*p - '0'); p++; }
			tokens[count].type = TOK_NUMBER;
			tokens[count].value = val;
			sprintf(tokens[count].text, "%d", val);
			count++;
			continue;
		}
		if (isalpha(*p) || *p == '_') {
            int len = 0;
            while (isalnum(*p) || *p == '_')
                tokens[count].text[len++] = *p++;
            tokens[count].text[len] = '\0';
            tokens[count].type      = TOK_IDENT;
            count++;
            continue;
        }
		if (*p == '"') {
            p++;
            int len = 0;
            while (*p && *p != '"')
                tokens[count].text[len++] = *p++;
            tokens[count].text[len] = '\0';
            tokens[count].type      = TOK_STRING;
            count++;
            if (*p == '"') p++;
            continue;
        }
        if (*p == '=' || *p == '!' || *p == '<' || *p == '>') {
            tokens[count].type     = TOK_SYMBOL;
            tokens[count].text[0]  = *p;
            if (*(p + 1) == '=') {
                tokens[count].text[1] = '=';
                tokens[count].text[2] = '\0';
                p += 2;
            } else {
                tokens[count].text[1] = '\0';
                p++;
            }
            count++;
            continue;
        }
		if (*p == '/' && *(p + 1) == '/') {
            while (*p && *p != '\n') p++;
            continue;
        }
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
            *p == '(' || *p == ')' || *p == ',' ||
            *p == '{' || *p == '}') {
            tokens[count].type     = TOK_SYMBOL;
            tokens[count].text[0]  = *p;
            tokens[count].text[1]  = '\0';
            count++;
            p++;
            continue;
        }
        fprintf(stderr, "lexer: unknown symbol '%c'\n", *p);
        exit(1);
 
	}
	return count;
}
