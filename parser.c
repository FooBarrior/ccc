#include <assert.h>

#include "lexer.h"

typedef enum{
	PRSRE_TERMINAL_NODE,
	PRSRE_BINARY_OP_NODE,
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

#define PRSR_MAX_BUF_SIZE 5000

char lastError[PRSR_MAX_BUF_SIZE];

NodePtr processErrorToken(LXR_TokenPtr t, const char *info){
	strcpy(lastError, info);
	return NULL;
}

typedef int PRSR_PriorityLevel;

PRSR_PriorityLevel priorities[] = {};

inline static bool isRoundBrase(LXR_TokenType t){
	return t == LXRE_LEFT_ROUND_BRACKET || t == LXRE_RIGHT_ROUND_BRACKET;
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

static NodePtr parseExpr(PRSR_PriorityLevel priority){
	static LXR_TokenPtr lastToken = NULL;
	PRSR_PriorityLevel lastPriority = priority;
	NodePtr root = NULL;
	NodePtr *readingNode = &root;
	while(1){
		LXR_TokenPtr t = lastToken == NULL ? lxr_nextToken() : lastToken;

		//read prefix ops here
		if(t->type == LXRE_LEFT_ROUND_BRACKET){
			NodePtr ln = parseExpr(1);
			*readingNode = ln;
			if(lastToken->type != LXRE_RIGHT_ROUND_BRACKET)
				return processErrorToken(t, "closing round bracket expected");
			lastToken = lxr_nextToken();
		}

		else if(!LXR_IS_IN_OP_CLASS(t, TERMINALS))
			return processErrorToken(t, "idnetifier or constant expected");

		else{
			if(lastPriority == priority){
				*readingNode = (NodePtr)initTermNode(NEW(TermNode), t);
				lastToken = lxr_nextToken();
			} else if(lastPriority > priority){
				NodePtr ln = parseExpr(lastPriority);
				*readingNode = ln;
			} else{
				assert(0); // wrong? cause we should already return
			}
		}

		//read postfix ops here

		if(LXR_IS_IN_OP_CLASS(t, TERMINALS) || (!LXR_IS_IN_OP_CLASS(t, BINARY_OPS) && LXR_IS_IN_OP_CLASS(t, PREFIX_OPS)))
			return processErrorToken(t, "unexpected token");

		if(!LXR_IS_IN_OP_CLASS(t, BINARY_OPS)){
			return root;
		}
		lastPriority = priorities[lastToken->type];
		if(lastPriority < priority){
			return root;
		}

		BinOpNodePtr n = initBinOpNode(NEW(BinOpNode), lastToken, root, NULL);
		root = (NodePtr)n;
		readingNode = &n->right;
	}
}

NodePtr prsr_parse(){
	return parseExpr(1);
}

#ifdef CCC_TEST
int main(){
	parseExpr(1);
}
#endif