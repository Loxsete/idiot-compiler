#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

AST parse(Token *tokens, int count);

char *read_file(const char *filename);

#endif 
