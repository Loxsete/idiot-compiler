#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"

void codegen(FILE *out, AST *nodes, int node_count);

#endif 
