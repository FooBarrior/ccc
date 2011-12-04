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
	if(t == NULL) strcpy(lastError, info);
	else sprintf(lastError, "%d:%d: %s", t->line, t->col, info);
	return NULL;
}

typedef int PRSR_PriorityLevel;

PRSR_PriorityLevel priorities[LXRE_TYPES_COUNT] = {0,0,0,0,0,0,0,0,
10,14,14,15,
15,15,
/*<<*/13,13,12,12,12,12,11,
9,8,7,6,5,2,
3,3,3,3,3,3,3,3,3,3,3,4, 0,};

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

LXR_TokenPtr token = NULL;

static NodePtr parseExpr(PRSR_PriorityLevel priority){
	LXR_TokenPtr t = token;
	token = lxr_nextToken();

	if(t == NULL)
		return processErrorToken(t, "unexpected end of file");

	// TODO read prefix ops here
	NodePtr ln = NULL;
	if(t->type == LXRE_LEFT_ROUND_BRACKET){
		ln = parseExpr(0);
		if(token->type != LXRE_RIGHT_ROUND_BRACKET)
			return processErrorToken(t, "closing round bracket expected");
		token = lxr_nextToken();
	}
	else if(!LXR_IS_IN_OP_CLASS(t, TERMINALS))
		return processErrorToken(t, "idnetifier or constant expected");
	else ln = initTermNode(NEW(TermNode), t);

	// TODO read postfix ops here

	if(token == NULL) return ln;
	PRSR_PriorityLevel pl = priorities[token->type];
	while(priority < pl){
		t = token;
		token = lxr_nextToken();
		ln = (NodePtr)initBinOpNode(NEW(BinOpNode), t, ln, parseExpr(pl));
		if(token == NULL) break;
		pl = priorities[token->type];
	}
	return ln;



/*

	static LXR_TokenPtr lastToken = NULL;
	PRSR_PriorityLevel prevPriority = priority, lastPriority = priority;
	NodePtr root = NULL;
	NodePtr *readingNode = &root;
	while(1){
		LXR_TokenPtr t = lastToken == NULL ? lxr_nextToken() : lastToken;
		if(t == NULL) return root;

		//read prefix ops here

		if(t->type == LXRE_LEFT_ROUND_BRACKET){
			NodePtr ln = parseExpr(100);
			*readingNode = ln;
			if(lastToken->type != LXRE_RIGHT_ROUND_BRACKET)
				return processErrorToken(t, "closing round bracket expected");
			lastToken = lxr_nextToken();
		}

		else if(!LXR_IS_IN_OP_CLASS(t, TERMINALS))
			return processErrorToken(t, "idnetifier or constant expected");

		else{
			if(lastPriority <= prevPriority){
				*readingNode = (NodePtr)initTermNode(NEW(TermNode), t);
				lastToken = lxr_nextToken();
				prevPriority = lastPriority;
			} else if(lastPriority > prevPriority){
				NodePtr ln = parseExpr(lastPriority);
				*readingNode = ln;

			} else{
				assert(0); // wrong? cause we should already return
			}
		}

		//read postfix ops here

		t = lastToken;
		if(t == NULL) return root;

		if(LXR_IS_IN_OP_CLASS(t, TERMINALS) || (!LXR_IS_IN_OP_CLASS(t, BINARY_OPS) && LXR_IS_IN_OP_CLASS(t, PREFIX_OPS)))
			return processErrorToken(t, "unexpected token");

		if(!LXR_IS_IN_OP_CLASS(t, BINARY_OPS)){
			return root;
		}
		prevPriority = lastPriority;
		lastPriority = priorities[lastToken->type];
		if(lastPriority < prevPriority){
			return root;
		}

		BinOpNodePtr n = initBinOpNode(NEW(BinOpNode), lastToken, root, NULL);
		root = (NodePtr)n;
		readingNode = &n->right;
		lastToken = NULL;
	}
*/
}

NodePtr prsr_parse(){
	token = lxr_nextToken();
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