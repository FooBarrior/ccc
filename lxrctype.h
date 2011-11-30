#include <ctype.h>

inline static bool isIdNondigit(char);
inline static bool isId(char);
inline static bool isOct(char);
inline static bool isSign(char);
inline static int xToInt(char);

bool isIdNondigit(char c){
	return isalpha(c) || c == '_';
}

bool isId(char c){
	return isIdNondigit(c) || isdigit(c);
}

bool isOct(char c){
	return c >= '0' && c <= '7';
}

bool isSign(char c){
	return c == '+' || c == '-';
}

int xToInt(char c){
	return isdigit(c) ? c - '0' : tolower(c) + 10 - 'a';
}

bool isNewline(char c){
	return c == '\r' || c == '\n' || c == '\v';
}