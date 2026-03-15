#ifndef AST_H
#define AST_H

typedef enum {
	TOK_NUMBER,
	TOK_IDENT,
	TOK_SYMBOL,
	TOK_STRING
} TokenType;

typedef enum {
    VAR_INT,
    VAR_CHAR,
    VAR_BOOL
} VarType;

typedef struct {
	TokenType type;
	char text[256];
	int value;
} Token;

typedef enum {
    AST_ASSIGN,
    AST_ASSIGN_STR,
    AST_PRINT,
    AST_PRINT_STR,
    AST_FUNC_DEF,
    AST_FUNC_END,
    AST_FUNC_CALL,
    AST_IF,
    AST_ELSE,
    AST_IF_END
} ASTType;

typedef struct {
	ASTType type;

	VarType var_type;
	char var[256]; // varible name, func name

	char left_str[256];
    char right_str[64];
    char op_sign[4];
    int  left;
    int  right;

    char args[8][64];
    int arg_count;

    char op[4];
    char cmp_left[64];
    char cmp_right[64];

    int if_id;

    int has_newline;
} AST;

#endif
