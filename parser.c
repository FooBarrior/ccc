#include <assert.h>

#include "lexer.h"

typedef enum{
	PRSRE_TERMINAL_NODE,
	PRSRE_BINARY_OP_NODE,
	PRSRE_UNARY_PREFIX_OP_NODE,
	PRSRE_UNARY_POSTFIX_OP_NODE,
	PRSRE_SUBSCRIPT_NODE,
	PRSRE_FUNC_CALL_NODE,
	PRSRE_TYPECAST_NODE,

	PRSRE_ERROR_NODE,
} PRSR_NodeType;

typedef struct{
	PRSR_NodeType type;
	LXR_TokenPtr t;
} PRSR_Node, *PRSR_NodePtr;

typedef struct PRSR_BinOpNode{
	PRSR_Node parent;
	PRSR_NodePtr left,right;
} PRSR_BinOpNode, *PRSR_BinOpNodePtr;
typedef PRSR_Node PRSR_TermNode, *PRSR_TermNodePtr;

typedef PRSR_TermNode TermNode;
typedef PRSR_TermNodePtr TermNodePtr;
typedef PRSR_BinOpNode BinOpNode, *BinOpNodePtr;
typedef PRSR_NodePtr NodePtr;
typedef PRSR_Node Node;

typedef int PRSR_PriorityLevel;

#define PRSR_MAX_BUF_SIZE 5000

char lastError[PRSR_MAX_BUF_SIZE];

static NodePtr processErrorToken(LXR_TokenPtr t, const char *info);

static inline TermNodePtr initTermNode(TermNodePtr n, LXR_TokenPtr t);
static inline BinOpNodePtr initBinOpNode(PRSR_BinOpNodePtr n, LXR_TokenPtr t, NodePtr l, NodePtr r);

static inline LXR_TokenPtr nextToken();
static NodePtr parseExpr(PRSR_PriorityLevel priority);

extern NodePtr prsr_parse();

PRSR_PriorityLevel priorities[LXRE_TYPES_COUNT] = {
	0,0,0,0,0,0,0,0, // unary
	10,14,14,15, // & + - *
	15,15, // / %
	13,13,12,12,12,12,11, // << >> < > <= >= ==
	9,8,7,6,5,2, // ^ | && ^^ || ,
	3,3,3,3,3,3,3,3,3,3,3,4, 0, // = += -= /= *= %= |= ^= <<= >>= ?   others are zero
};

NodePtr processErrorToken(LXR_TokenPtr t, const char *info){
	if(t == NULL) strcpy(lastError, info);
	else sprintf(lastError, "%d:%d: %s", t->line, t->col, info);
	return NULL;
}

TermNodePtr initTermNode(TermNodePtr n, LXR_TokenPtr t){
	n->type = PRSRE_TERMINAL_NODE;
	n->t = t;
	return n;
}
BinOpNodePtr initBinOpNode(PRSR_BinOpNodePtr n, LXR_TokenPtr t, NodePtr l, NodePtr r){
	n->parent.type = PRSRE_BINARY_OP_NODE;
	n->parent.t = t;
	n->left = l;
	n->right = r;
	return n;
}

LXR_TokenPtr token = NULL;

LXR_TokenPtr nextToken(){
	LXR_TokenPtr t = lxr_nextToken();
	assert(t != NULL);
	return t;
}

NodePtr parseExpr(PRSR_PriorityLevel priority){
	LXR_TokenPtr t = token;
	token = nextToken();

	if(t->type == LXRE_EOF)
		return processErrorToken(t, "unexpected end of file");

	// TODO read prefix ops here
	
	NodePtr root = NULL;
	if(t->type == LXRE_LEFT_ROUND_BRACKET){
		root = parseExpr(0);
		if(token->type != LXRE_RIGHT_ROUND_BRACKET)
			return processErrorToken(t, "closing round bracket expected");
		token = nextToken();
	}
	else if(!LXR_IS_IN_OP_CLASS(t, TERMINALS))
		return processErrorToken(t, "identifier, constant, or opening round bracket expected");
	else root = initTermNode(NEW(TermNode), t);

	// TODO read postfix ops here

	PRSR_PriorityLevel pl = priorities[token->type];
	while(priority < pl){
		t = token;
		token = nextToken();
		if(LXR_IS_IN_OP_CLASS(t, ASSIGN_OPS)) pl--;
		root = (NodePtr)initBinOpNode(NEW(BinOpNode), t, root, parseExpr(pl));
		
		pl = priorities[token->type];
	}
	return root;

}

NodePtr prsr_parse(){
	token = nextToken();
	return parseExpr(0);
}

#ifdef CCC_TEST
extern char* lxr_opTokenValues[LXRE_PUNCTUATORS_COUNT];
void printToken(LXR_TokenPtr t){
	char *s[] = {"identifier", "int", "float", "string", "invalid token"};
	if(LXR_IS_IN_OP_CLASS(t, TERMINALS) || t->type == LXRE_TOKEN_INVALID)
		printf("%s ('%s')", s[t->type - LXRE_TERMINALS_START], LXR_GETBUF(t));
	else{
		assert(t->type < LXRE_PUNCTUATORS_COUNT);
		printf("\"%s\"", lxr_opTokenValues[t->type]);
	}
}
#include <stdio.h>

void printJSON(NodePtr n){
	if(n == NULL){ printf("\n\n\"NULL found. last error: %s\"\n", lastError); return;}
	if(n->type == PRSRE_TERMINAL_NODE){
		printf("\"");
		printToken(n->t);
		printf("\"");
	} else{
		printf("{\"%s\":[", lxr_opTokenValues[n->t->type]);
		printJSON(((BinOpNodePtr)n)->left);
		printf(",");
		printJSON(((BinOpNodePtr)n)->right);
		printf("]}");
	}
}
int main(){
	freopen("parser.t", "r", stdin);
	lxr_initLexer(stdin);
	printJSON(prsr_parse());

}
#endif