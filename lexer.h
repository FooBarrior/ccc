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
	LXRE_POSTFIX_OPS_START,

	/* -> */ LXRE_ARROW = LXRE_POSTFIX_OPS_START,
	/* .  */ LXRE_DOT,

	/* (  */ LXRE_LEFT_ROUND_BRACKET,
	/* [  */ LXRE_LEFT_SQUARE_BRACKET,

	LXRE_PREFIX_OPS_START,

	/* ++ */ LXRE_INCREASE = LXRE_PREFIX_OPS_START,
	/* -- */ LXRE_DECREASE,

	LXRE_POSTFIX_OPS_END = LXRE_DECREASE,

	/* !  */ LXRE_NOT,
	/* ~  */ LXRE_TILDA,

	LXRE_BINARY_OPS_START,

	/* & 6 */ LXRE_AND = LXRE_BINARY_OPS_START,
	/* + 2 */ LXRE_ADD,
	/* - 2 */ LXRE_SUB,
	/* * 1 */ LXRE_MULT,

	LXRE_PREFIX_OPS_END = LXRE_MULT,

	/* / 1 */ LXRE_DIV,
	/* % 1 */ LXRE_MOD,

	/* << 3*/ LXRE_SHL,
	/* >> 3*/ LXRE_SHR,
	/* < 4 */ LXRE_LT,
	/* > 4 */ LXRE_GT,
	/* <= 4*/ LXRE_LT_EQUAL,
	/* >= 4*/ LXRE_GT_EQUAL,
	/* == 5*/ LXRE_EQ,
	
	/* ^ 7 */ LXRE_XOR,
	/* | 8 */ LXRE_OR,

	/* && 9*/ LXRE_LOGICAL_AND,
	/* ^^ 10*/ LXRE_LOGICAL_XOR,
	/* || 11*/ LXRE_LOGICAL_OR,
	/* , 14 */ LXRE_COMMA,

	LXRE_ASSIGN_OPS_START,

	/* =  13*/ LXRE_ASSIGN = LXRE_ASSIGN_OPS_START,
	/* += */ LXRE_ADD_ASSIGN,
	/* -= */ LXRE_SUB_ASSIGN,
	/* /= */ LXRE_DIV_ASSIGN,
	/* *= */ LXRE_MULT_ASSIGN,
	/* %= */ LXRE_MOD_ASSIGN,
	/* &= */ LXRE_AND_ASSIGN,
	/* |= */ LXRE_OR_ASSIGN,
	/* ^= */ LXRE_XOR_ASSIGN,
	/* <<= */ LXRE_SHL_ASSIGN,
	/* >>= */ LXRE_SHR_ASSIGN,

	LXRE_ASSIGN_OPS_END = LXRE_SHR_ASSIGN,

	LXRE_BINARY_OPS_END = LXRE_SHR_ASSIGN,

	/* ? */ LXRE_QUESTION,
	/* : */ LXRE_COLON,
	/* ... */ LXRE_ELLIPSIS,
	/* ; */ LXRE_SEMICOLON,

	/* { */ LXRE_LEFT_CURLY_BRACKET,
	/* } */ LXRE_RIGHT_CURLY_BRACKET,
	/* ] */ LXRE_RIGHT_SQUARE_BRACKET,
	/* ) */ LXRE_RIGHT_ROUND_BRACKET,

	LXRE_PUNCTUATORS_COUNT,

	LXRE_TERMINALS_START = LXRE_PUNCTUATORS_COUNT,

	LXRE_FLOAT_CONST = LXRE_TERMINALS_START,
	LXRE_INT_CONST,
	LXRE_IDENTIFIER,
	LXRE_STRING_CONST,

	LXRE_TERMINALS_END = LXRE_STRING_CONST,

	LXRE_TOKEN_INVALID,

	LXRE_TYPES_COUNT,
	
} LXR_TokenType;

#define LXR_IS_IN_OP_CLASS(t, OP_CLASS) \
	(t->type >= LXRE_##OP_CLASS##_START && t->type <= LXRE_##OP_CLASS##_END)

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
extern void lxr_initLexer(FILE*);
extern void lxr_deinitializeLexer();
