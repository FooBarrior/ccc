#include <stdlib.h>
#include <assert.h>
//#include <math.h>
#include "lexer.h"
#include "lxrctype.h"

typedef LXR_Token Token;
typedef LXR_TokenPtr TokenPtr;
typedef LXR_StrToken StrToken;
typedef LXR_StrTokenPtr StrTokenPtr;
typedef LXR_IntToken IntToken;
typedef LXR_IntTokenPtr IntTokenPtr;
typedef LXR_FloatToken FloatToken;
typedef LXR_FloatTokenPtr FloatTokenPtr;

#define LXR_MAX_WORD_SIZE 500
#define LXR_MAX_STR_WIDTH 4095

Lexer lexer;
char lxr_buff[LXR_MAX_STR_WIDTH];
char *buff = lxr_buff;

#define LXR_GETCHAR (lexer.col++, getc(lexer.f))
#define LXR_UNGETC(c) (ungetc(c, lexer.f), lexer.col--)

#define LXR_FEED(c, len) (buff[len++] = c, c = LXR_GETCHAR)
#define LXR_FEED_WHILE(c, cond, len) while(cond)LXR_FEED(c, len)
#define LXR_FEED_IF(c, cond, len) ((cond) ? (LXR_FEED(c, len), true) : false)

#define LXR_THROW_ERROR_FMT(fmt, ...) { \
		sprintf(buff, fmt, __VA_ARGS__); \
		lexer.last = LXR_GETCHAR; \
		return newErrorToken(); \
	}
	

inline static void provokeLineFeed();

inline static TokenPtr initToken(TokenPtr, LXR_TokenType);
 static TokenPtr initStrToken(StrTokenPtr, LXR_TokenType, int);
static TokenPtr initIntToken(IntTokenPtr, int);
inline static TokenPtr initFloatToken(FloatTokenPtr, int);
static TokenPtr newErrorToken();

static TokenPtr readNum(char);
static TokenPtr readId();
//static TokenPtr readPreproc();

static bool missComments();

static TokenPtr throwErrorFmtExpect(char expect[], char but, int len){
	char p[len+1];
	memcpy(p, buff, len);
	p[len] = 0;
	char butStr[2] = {but, 0};
	LXR_THROW_ERROR_FMT("After %s: %s expected, but '%s'(%hhu) found", p, expect, isblank(but) ? butStr : "\\n", but)
}

void provokeLineFeed(){
	lexer.str++;
	lexer.col = 0;
}

TokenPtr initToken(TokenPtr tok, LXR_TokenType type){
	tok->type = type;
	tok->line = lexer.str;
	tok->col = lexer.start;
	return tok;
}

TokenPtr initStrToken(StrTokenPtr tok, LXR_TokenType type, int len){
	initToken((TokenPtr)tok, type);
	tok->buff = malloc(len + 1);
	for(int i = 0; i < len; i++) tok->buff[i] = buff[i];
	tok->buff[len] = 0;
	tok->length = len;
	return (TokenPtr)tok;
}

TokenPtr initIntToken(IntTokenPtr tok, int len){
	char *proto = buff;
	initStrToken((StrTokenPtr)tok, LXRE_INT_CONST, len);
	int mult = proto[0] == '0' ? 8 : 10, start = (int)proto[0] == '0';
	if(mult == 8 && tolower(proto[1]) == 'x'){
		mult *= 2;
		start++;
	}
	unsigned result = 0;
	for(int i = start; i < len; i++){
		result *= mult;
		result += xToInt(proto[i]);
	}
	tok->value = result;
	return (TokenPtr)tok;
}

TokenPtr initFloatToken(FloatTokenPtr tok, int len){
	initStrToken((StrTokenPtr)tok, LXRE_FLOAT_CONST, len);

#ifdef LXR_DONT_USE_STDLIB_ATOF
	char *proto = buff;
	bool isHex = tolower(proto[1]) == 'x';
	assert(proto[0] == '0' || !isHex);
	if(isHex) proto += 2, len -= 2;

	int dotPos = 0;
	for(; dotPos < len && proto[dotPos] != '.'; dotPos++);
	//assert(dotPos == len); && epos == len

	char e = isHex ? 'p' : 'e';
	int ePos = 0;
	for(; ePos < len && tolower(proto[ePos]) != e; ePos++);
	assert(ePos != len || !isHex && dotPos != len);
	assert(dotPos == len || dotPos < ePos);

	double res = 0;
	int mult = isHex ? 16 : 10, base = isHex ? 2 : 10;
	for(int i = ePos - 1; i > dotPos; i--)
		res /= mult += xToInt(proto[i]);
	
	for(int i = min(dotPos, ePos) -1; i >= 0; i--)
		res /= mult += xToInt(proto[i]);
	//это всё не компилируется
#else
	tok->value = atof(tok->parent.buff);
#endif
	return (TokenPtr)tok;
}

TokenPtr newErrorToken(){
	return initStrToken(NEW(StrToken), LXRE_TOKEN_INVALID, strlen(buff));
}

#define LXR_THROW_ERROR_FMT_EXPECT(expect, but) return throwErrorFmtExpect(expect, but, len);

TokenPtr readNum(char c){
	int len = c != '.';
	TokenPtr tok = NULL;
	bool isHexNum = c == '0' && lexer.last == 'x';
	buff[0] = c;

	if(isHexNum){
		/*
		0x(\h*\.)?\h*(p[+-]?\d+)?
		*/
		buff[1] = 'x';
		len++;
		c = LXR_GETCHAR;
		if(!isxdigit(c) && c != '.')
			LXR_THROW_ERROR_FMT_EXPECT("hex digit or '.'", c)
		LXR_FEED_WHILE(c, isxdigit(c), len);
		if(LXR_FEED_IF(c, c == '.', len)){
			if(!isxdigit(c))
				LXR_THROW_ERROR_FMT_EXPECT("hex digit or 'p'", c)

			LXR_FEED_WHILE(c, isxdigit(c), len);

			if(LXR_FEED_IF(c, tolower(c) == 'p', len)){
				LXR_FEED_IF(c, isSign(c), len);

				if(!isdigit(c))
					LXR_THROW_ERROR_FMT_EXPECT("digit", c);

				LXR_FEED_WHILE(c, isdigit(c), len);
			}
			tok = initFloatToken(NEW(FloatToken), len);
		} else{
			tok = initIntToken(NEW(IntToken), len);
		}
	} else if(c == '0' && isdigit(lexer.last)){
		if(!isOct(lexer.last))
			LXR_THROW_ERROR_FMT_EXPECT("octal digit", c);

		buff[1] = lexer.last;
		c = LXR_GETCHAR;
		LXR_FEED_WHILE(c, isOct(c), len);
		if(isdigit(c))
			LXR_THROW_ERROR_FMT_EXPECT("octal digit", c);
		tok = initIntToken(NEW(IntToken), len);
	} else if(!isalpha(lexer.last) || lexer.last == 'e'){
		if(c != '.'){
			c = lexer.last;
			LXR_FEED_WHILE(c, isdigit(c), len);
		} else LXR_UNGETC(lexer.last);
		if(c == '.' || tolower(c) == 'e'){
			if(LXR_FEED_IF(c, c == '.', len)){
				LXR_FEED_WHILE(c, isdigit(c), len);
			}
			if(LXR_FEED_IF(c, tolower(c) == 'e', len)){
				LXR_FEED_IF(c, isSign(c), len);
				if(!isdigit(c))
					LXR_THROW_ERROR_FMT_EXPECT("digit", c);

				LXR_FEED_WHILE(c, isdigit(c), len);
			}
			tok = initFloatToken(NEW(FloatToken), len);
		} else{
			tok = initIntToken(NEW(IntToken), len);
		}
	} else
		LXR_THROW_ERROR_FMT_EXPECT("digit or e", c);
	lexer.last = c;
	return tok;
}

TokenPtr readId(){
	char c = lexer.last;
	int len = 0;
	LXR_FEED_WHILE(c, isId(c), len);
	lexer.last = c;
	return initStrToken(NEW(StrToken), LXRE_IDENTIFIER, len);
}

bool missComments(){
	while(lexer.last == '/'){
		char c = LXR_GETCHAR;
		switch(c){
			case '*':
				lexer.last = LXR_GETCHAR;
				while(1){
					c = lexer.last;
					lexer.last = LXR_GETCHAR;
					if(c == '*' && lexer.last == '/') break;
					if(lexer.last == EOF) return true;
				}
				lexer.last = LXR_GETCHAR;
				return true;

			case '/':
				fgets(buff, LXR_MAX_STR_WIDTH, lexer.f);
				provokeLineFeed();
				lexer.last = LXR_GETCHAR;
				return true;

			default:
				lexer.last = '/';
				LXR_UNGETC(c);
				return false;
		}
	}
	return false;
}

LXR_TokenPtr lxr_nextToken(){
	char c = lexer.last;

	do{
		c = lexer.last;
		
		while(isspace(c)){
			if(!isblank(c)) provokeLineFeed();
			c = LXR_GETCHAR;
		}
		if(c == EOF) return NULL;
		lexer.last = c;
	} while(missComments());

	lexer.start = lexer.col;

	if(isdigit(c) || c == '.'){
		lexer.last = tolower(LXR_GETCHAR);
		if(c == '.' && !isdigit(lexer.last)){
			if(lexer.last == '.'){
				lexer.last = LXR_GETCHAR;
				if(lexer.last != '.')
					LXR_THROW_ERROR_FMT("Unrecognized token '..'. Did you mean '...'?%c", 0)
				lexer.last = LXR_GETCHAR;
				return initToken(NEW(Token), LXRE_ELLIPSIS);
			}
			else return initToken(NEW(Token), LXRE_DOT);
		}
		return readNum(c);
	} 
	else if(isIdNondigit(c)) return readId();
	else{
		LXR_TokenType t = LXRE_TOKEN_INVALID;
		lexer.last = LXR_GETCHAR;
		bool q = c == lexer.last, w = lexer.last == '=', e = false;

#define LXR_OP1(x, a, b, c) \
	case x: \
		t = q ? LXRE_##a : w ? LXRE_##b##_##c : LXRE_##b;\
		e = q||w; \
		break

#define LXR_OP2(x, a) \
	case x: \
		t = w ? LXRE_##a##_ASSIGN : LXRE_##a; \
		e = w; \
		break

#define LXR_OP(x, a) case x: t = a; break
			
		switch(c){
				LXR_OP1('+', INCREASE, ADD, ASSIGN);
				LXR_OP1('-', DECREASE, SUB, ASSIGN);
				LXR_OP1('&', LOGICAL_AND, AND, ASSIGN);
				LXR_OP1('|', LOGICAL_OR, OR, ASSIGN);
				LXR_OP1('^', LOGICAL_XOR, XOR, ASSIGN);
				LXR_OP1('<', SHL, LT, EQUAL);
				LXR_OP1('>', SHR, GT, EQUAL);

				LXR_OP2('/', DIV);
				LXR_OP2('*', MULT);
				LXR_OP2('%', MOD);
				case '=':
					t = w ? LXRE_EQ : LXRE_ASSIGN;
					e = w;
					break;
								
				LXR_OP('.', LXRE_DOT);
				LXR_OP(',', LXRE_COMMA);
				LXR_OP('!', LXRE_NOT);
				LXR_OP('{', LXRE_LEFT_CURLY_BRACKET);
				LXR_OP('}', LXRE_RIGHT_CURLY_BRACKET);
				LXR_OP('[', LXRE_LEFT_SQUARE_BRACKET);
				LXR_OP(']', LXRE_RIGHT_SQUARE_BRACKET);
				LXR_OP('(', LXRE_LEFT_ROUND_BRACKET);
				LXR_OP(')', LXRE_RIGHT_ROUND_BRACKET);
				LXR_OP(';', LXRE_SEMICOLON);
				LXR_OP('~', LXRE_TILDA);
				LXR_OP(':', LXRE_COLON);
				LXR_OP('?', LXRE_QUESTION);

				default:
					LXR_THROW_ERROR_FMT("Unrecognized symbol '%c'", c);
		}
		if(e){
			lexer.last = LXR_GETCHAR;
			if((c == '<' || c == '>') && lexer.last == '=')
				t = c == '<' ? LXRE_SHL_ASSIGN : LXRE_SHR_ASSIGN;
		} else if (c == '-' && lexer.last == '>'){
			lexer.last = LXR_GETCHAR;
			t = LXRE_ARROW;
		}

		return initToken(NEW(Token), t);
	}
}

void lxr_initLexer(FILE *f){
	lexer.f = f;
	lexer.str = 1;
	lexer.col = 0;
	lexer.last = LXR_GETCHAR;
}

void lxr_deinitializeLexer(){
	fclose(lexer.f);
}

char* lxr_opTokenValues[LXRE_PUNCTUATORS_COUNT] = {
	"(", "[", "++", "--", "!", "~", "&", "+", "-", "*",
	"|", "^", "/", "%", "&&", "||", "^^",
	"<", ">", "<=", ">=", "<<", ">>", "==", ",",
	"=", "+=", "-=", "/=", "*=", "%=", "&=", "|=", "^=",
	"<<=", ">>=",
	"->", ".", "...", ";", "?", ":",
	"{", "}", "]", ")",
};

int lxr_runtest(char *filename, FILE *f, bool printDebug){
	if(f == NULL){
		printf("%s: no such file or directory\n", filename);
		exit(1);
	}
	lxr_initLexer(f);
	LXR_TokenPtr t = NULL;
	while((t = lxr_nextToken()) != NULL) if(printDebug){
		printf("%s:%d:%d: ", filename, t->line, t->col);
		switch(t->type){
			case LXRE_IDENTIFIER:
				printf("identifier | '%s'", LXR_GETBUF(t));
				break;
			case LXRE_INT_CONST:
				printf("int | '%s' | %d", LXR_GETBUF(t), LXR_GET_INT_VAL(t));
				break;
			case LXRE_FLOAT_CONST:
				printf("float | '%s' | %.12f", LXR_GETBUF(t), LXR_GET_FLOAT_VAL(t));
				break;
			case LXRE_TOKEN_INVALID:
				printf("Invalid token | %s", LXR_GETBUF(t));
				break;
			default:
				assert(t->type < LXRE_PUNCTUATORS_COUNT);
				printf("operator token | '%s'", lxr_opTokenValues[t->type]);
		}
		printf("\n");
	}
	lxr_deinitializeLexer();
	return 0;
}

#ifdef CCC_TEST
int main(int argc, char **argv){
	for(int i = 1; i < argc; i++){
		bool printDebug = strcmp(argv[i], "--no-debug-print");
		if(!printDebug) i++;
		lxr_runtest(argv[i], fopen(argv[i], "r"), printDebug);
	}
	return 0;
}
#endif