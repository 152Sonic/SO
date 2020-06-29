#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int seg=0;
int ctrc=0;

void sigint_handler(int sig){
	ctrc++;
	printf("seg=%d\n",seg);
}
void sigquit_handler(int sig){
    printf("ctr+c=%d\n",ctrc);
	exit(0);
}

void sigchld_handler(int sig){
	int status;
	printf("im out\n");
	printf("wait -> %d\n",wait(&status));
}

int pid;

void sigalrm_handler(int sig){
	int status;
    printf("ola\n");
    kill(pid,SIGKILL);
	printf("wait -> %d\n",wait(&status));
}

int main(){
	signal(SIGALRM,sigalrm_handler);

	signal(SIGCHLD,sigchld_handler);
	int status;
	alarm(10);

	switch((pid=fork())){
		case -1:
			perror("fork");
			return -1;
		case 0:
			printf("filho\n");
			printf("%d\n", getpid());
			sleep(5);
			_exit(0);
		default:
			break;
	}

	sleep(50);

	return 0;
}
