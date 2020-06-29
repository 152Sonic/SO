#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
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

#define MAX_SIZE_BUFFER 1024


void sigalrm_handler(int sig){
    int status;
    printf("ola\n");
}




int main(){
	int inat[3][2];
    int p[3][2];
    int res=0;
    int status;
	int pid;
	int N=3;
	char cmd[10][10]={"ls","uniq","wc"};
    //char cmd[10][10]={"cat","cat","wc"};
    int i;
    int o;
    char buffer[MAX_SIZE_BUFFER];
	
	for(i=0;i<N;i++){
		if(i==0){
			if(pipe(p[i])==-1){
                perror("Pipe");
                return -1;
            }
			
    		switch((pid=fork())){
               	case -1:
                    perror("Fork");
                 	return -1;
                case 0:
             		close(p[i][0]);
                   	dup2(p[i][1],1);
                  	close(p[i][1]);

                     res=execlp(cmd[i],cmd[i],NULL);
                   	_exit(0);
                            
    			default:
                    printf("Forked\n");
    				close(p[i][1]);            
            }
		}
		
		else if(i==N-1){
			switch((pid=fork())){
                case -1:
                    perror("Fork");
                    return -1;
                case 0:
                    dup2(inat[i-1][0],0);
                    close(inat[i-1][0]);

                    res=execlp(cmd[i],cmd[i],NULL);
                   _exit(0);

                default:
                    printf("Forked\n");
                }
	
		}
		
		
		else{
			if(pipe(p[i])==-1){
                perror("Pipe");
                return -1;
            }

            switch((pid=fork())){
                case -1:
                    perror("Fork");
                    return -1;
                case 0:
                    dup2(inat[i-1][0],0);
                    close(inat[i-1][0]);

                    close(p[i][0]);
                    dup2(p[i][1],1);
                    close(p[i][1]);

                    res=execlp(cmd[i],cmd[i],NULL);
                    _exit(0);

                default:
                    printf("Forked\n");
                    close(p[i][1]);
            }
		}

        //criador de pipes de verificação de inatividade
        if(i<N-1){
            if(pipe(inat[i])==-1){
                perror("Pipe de inatividade");
                return -1;
            }

            switch((pid=fork())){
                case -1:
                    perror("Fork");
                    return -1;
                case 0:
                    signal(SIGALRM,sigalrm_handler);
                    //while read -> write
                    alarm(4);
                    while((o=read(p[i][0],buffer,MAX_SIZE_BUFFER))>0){
                        alarm(0);
                        write(inat[i][1],buffer,o);
                    }

                    close(p[i][0]);
                    
                    _exit(0);

                default:
                    close(inat[i][1]);
            }
        }

	}
	
	for(i=0;i<2*N-1;i++){
		wait(&status);
	}
    printf("sair\n");
	return 0;
}


