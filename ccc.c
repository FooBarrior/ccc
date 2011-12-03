#include "lexer.h"

int main(int argc, char *argv[]){
	char *read_args = "reading arguments...\n";
	char *check_arg_v = "checking argument 0 for -v\n";
	bool _v_found = false;
	if(strcmp(argv[0], "-v")){
		printf(read_args);
		printf(check_arg_v);
		printf("key -v found\n");

		_v_found = true;
	}
	if(_v_found) printf("nothing to do. veaving...\n");
	return 0;
}