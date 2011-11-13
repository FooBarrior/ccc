#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define NEW(type) ((type*)malloc(sizeof(type)))
#define NEW_SEQ(type, len) ((type*)malloc(sizeof(type)*len))

typedef struct{
	FILE *f;
	int str, start, col; //so, end of token is at [str:start]
	char last;
} Lexer, *LexerPtr;

typedef enum LXR_TokenType{

	LXRE_DOT,
	LXRE_COMMA,
	LXRE_NOT,
	LXRE_LEFT_CURLY_BRACKET,
	LXRE_RIGHT_CURLY_BRACKET,
	LXRE_LEFT_SQUARE_BRACKET,
	LXRE_RIGHT_SQUARE_BRACKET,
	LXRE_LEFT_ROUND_BRACKET,
	LXRE_RIGHT_ROUND_BRACKET,
	LXRE_SEMICOLON,
	LXRE_TILDA,
	LXRE_COLON,
	LXRE_QUESTION,

	LXRE_ADD,
	LXRE_SUB,
	LXRE_ASSIGN,
	LXRE_LT,
	LXRE_GT,
	LXRE_AND,
	LXRE_OR,
	LXRE_XOR,
	LXRE_DIV,
	LXRE_MULT,
	LXRE_MOD,

	LXRE_ADD_ASSIGN,
	LXRE_SUB_ASSIGN,
	LXRE_INCREASE,
	LXRE_DECREASE,

	LXRE_AND_ASSIGN,
	LXRE_OR_ASSIGN,
	LXRE_XOR_ASSIGN,
	LXRE_LOGICAL_AND,
	LXRE_LOGICAL_OR,
	LXRE_LOGICAL_XOR,

	LXRE_EQ,
	LXRE_LT_EQUAL,
	LXRE_GT_EQUAL,
	LXRE_SHL,
	LXRE_SHR,
	LXRE_SHL_ASSIGN,
	LXRE_SHR_ASSIGN,

	LXRE_DIV_ASSIGN,
	LXRE_MULT_ASSIGN,
	LXRE_MOD_ASSIGN,

	LXRE_ARROW,

	LXRE_FLOAT_CONST,
	LXRE_INT_CONST,
	LXRE_IDENTIFIER,

	LXRE_TOKEN_INVALID,
	
} LXR_TokenType;

#define LXR_OP_TOKEN_COUNT 45

typedef struct{
	LXR_TokenType type;
	int line, col;
} LXR_Token, *LXR_TokenPtr;

typedef struct{
	LXR_Token parent;
	char *buff;
	int length;
} LXR_StrToken, *LXR_StrTokenPtr;

typedef struct{
	LXR_StrToken parent;
	unsigned value;
} LXR_IntToken, *LXR_IntTokenPtr;

typedef struct{
	LXR_StrToken parent;
	double value;
} LXR_FloatToken, *LXR_FloatTokenPtr;

#define LXR_GETBUF(t) (((LXR_StrTokenPtr)t)->buff)
#define LXR_GET_INT_VAL(t) (((LXR_IntTokenPtr)t)->value)
#define LXR_GET_FLOAT_VAL(t) (((LXR_FloatTokenPtr)t)->value)

extern LXR_TokenPtr lxr_nextToken();
extern void lxr_initLexer(char *filename);
