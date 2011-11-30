#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv){
	if(argc < 2){
		puts("No program specified for processing");
		return 0;
	}
	char *eargv[argc];
	for(int i = 1; i < argc; i++) eargv[i-1] = argv[i];
	eargv[argc-1] = NULL;
	struct timeval tv1, tv2, tvres;
	pid_t pid = fork();
	if(!pid){
		freopen("/dev/null", "w", stdout);
		execv(argv[1], eargv);
	} else if(pid < 0)
		printf("fork failed\n");
	else{
		gettimeofday(&tv1, NULL);
		wait(0);
		gettimeofday(&tv2, NULL);
		timersub(&tv2, &tv1, &tvres);
		if(fprintf(stderr, "%d.%d\n", (int)tvres.tv_sec, (int)tvres.tv_usec) < 0) printf("Cant output time!\n");
	}
	return 0;
}
