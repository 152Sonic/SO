#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "argus.h"


int ti=-1;
int te=-1;
int tarefa_index=0;
int sizepids=0;
int start_tarefas=0;
int *pids=NULL;



struct logidx{
	int tarefa;					//numero da tarefa
	char estado[20];			//estado no qual terminou
	char valorc[COMMAND_SIZE];	//comando executado
	int index;					//indice do output no ficheiro log
	int size;					//tamanho do output
};

char **comandos;



void writemsg(char* string){
	write(1,string,strlen(string));
}
void writeerror(char* string){
	write(2,string,strlen(string));
}

/* sinal para fechar o servidor (force close)
*/
void sigint_handler(int sig){//melhorar o tratamento
	unlink(FIFO1);
	unlink(FIFO2);
	for (int i = start_tarefas; i < tarefa_index; i++){
		if(pids[i]>0){
			kill(pids[i],SIGUSR2);
			pids[i]=-1;	
		}
	}
	writemsg("\n");
	_exit(0);
}




int poslogidx;							//posicao da entrada da tarefa x no ficheiro log.idx

int pidsexecucao[COMMAND_SIZE]={0};		//pids dos processos que esão a executar os comandos individuais
int pidsaver =0;						//pid do processo que vai gravar o output (o primeiro a receber o sianl)

void sigalrm_execucao_handler(int sig){	//melhorar o tratamento
	if(pidsaver>0) kill(pidsaver,SIGKILL);
	for (int i = 0; i < COMMAND_SIZE; ++i){
		if(pidsexecucao[i]>0) kill(pidsexecucao[i],SIGKILL);
	}

	struct logidx slogidx;
	int flogidx;

	if((flogidx=open("log.idx",O_CREAT | O_RDWR,  0666))==-1){
		perror("open log.idx");
	}
	lseek(flogidx,poslogidx,SEEK_SET);
	read(flogidx,&slogidx,sizeof(struct logidx));
	lseek(flogidx,poslogidx,SEEK_SET);
	strcpy(slogidx.estado,"max execucao");
	if(write(flogidx,&slogidx,sizeof(struct logidx))==-1){
		perror("write log.idx");
	}

	close(flogidx);

}

void sigusr1_inatividade_handler(int sig){//melhorar o tratamento
	if(pidsaver>0) kill(pidsaver,SIGKILL);
	for (int i = 0; i < COMMAND_SIZE; ++i){
		if(pidsexecucao[i]>0) kill(pidsexecucao[i],SIGKILL);
	}

	struct logidx slogidx;
	int flogidx;

	if((flogidx=open("log.idx",O_CREAT | O_RDWR,  0666))==-1){
		perror("open log.idx");
	}
	lseek(flogidx,poslogidx,SEEK_SET);
	read(flogidx,&slogidx,sizeof(struct logidx));
	lseek(flogidx,poslogidx,SEEK_SET);
	strcpy(slogidx.estado,"max inatividade");
	if(write(flogidx,&slogidx,sizeof(struct logidx))==-1){
		perror("write log.idx");
	}

	close(flogidx);
}

void sigusr2_terminar_handler(int sig){			//handler do SIGUSR2 -> sinal que o processo encarregue do executar recebe da main quando é pedido para terminar um processo
	if(pidsaver>0) kill(pidsaver,SIGKILL);

	for (int i = 0; i < COMMAND_SIZE; ++i){
		if(pidsexecucao[i]>0) kill(pidsexecucao[i],SIGKILL);
	}

	struct logidx slogidx;
	int flogidx;

	if((flogidx=open("log.idx",O_CREAT | O_RDWR,  0666))==-1){
		perror("open log.idx");
	}
	lseek(flogidx,poslogidx,SEEK_SET);
	read(flogidx,&slogidx,sizeof(struct logidx));
	lseek(flogidx,poslogidx,SEEK_SET);
	strcpy(slogidx.estado,"terminada");
	if(write(flogidx,&slogidx,sizeof(struct logidx))==-1){
		perror("write log.idx");
	}

	close(flogidx);
}

void myredirect(char *tipored,char *ficheiro){
	int fd;
	if(strcmp(tipored,"<")){
		fd=open(ficheiro,O_RDONLY);
		dup2(fd,0);
		close(fd);
	}
	else if(strcmp(tipored,">")){
		fd=open(ficheiro,O_CREAT | O_TRUNC | O_WRONLY, 0666);
		dup2(fd,1);
		close(fd);		
	}
	else if(strcmp(tipored,">>")){
		fd=open(ficheiro,O_CREAT | O_APPEND | O_WRONLY, 0666);
		dup2(fd,1);
		close(fd);			
	}
	else if(strcmp(tipored,"2>")){
		fd=open(ficheiro,O_CREAT | O_TRUNC | O_WRONLY, 0666);
		dup2(fd,2);
		close(fd);			
	}
	else{
		fd=open(ficheiro,O_CREAT | O_APPEND | O_WRONLY, 0666);
		dup2(fd,2);
		close(fd);			
	}
}

int mysystem(char *comando){				//funcao que executa um comando individual
	char *args[COMMAND_SIZE];
	int i=0;
	char *parte = strtok(comando," ");
	while(parte!=NULL){
		if(!strcmp(parte,"<") || !strcmp(parte,">") ||!strcmp(parte,">>") ||!strcmp(parte,"2>") ||!strcmp(parte,"2>>")){
			char tipored[4];
			strcpy(tipored,parte);
			parte=strtok(NULL," ");
			//myredirect(tipored,parte);
		}else{
			args[i] = parte;
			parte=strtok(NULL," ");
			i++;
		}
	}
	args[i]='\0';


	int exec_ret=execvp(args[0],args);		
	perror("exec");
	_exit(exec_ret);

}

void sigalrm_pipeinativo_handler(int sig){		//envia sinal USR1 ao
	kill(getppid(),SIGUSR1);
}

int executar(char *comando){
	struct logidx slogidx;
	slogidx.tarefa=tarefa_index;	
	strcpy(slogidx.estado,"undifined");
	strcpy(slogidx.valorc,comando);
	int flogidx;
	if((flogidx=open("log.idx",O_CREAT | O_APPEND | O_WRONLY,  0666))==-1){
		perror("open log.idx");
		return -1;
	}
	if(write(flogidx,&slogidx,sizeof(struct logidx))==-1){
		perror("write log.idx");
	}
	poslogidx=lseek(flogidx,-1*sizeof(struct logidx),SEEK_END);
	close(flogidx);

	
	strcpy(slogidx.estado,"concluida");


	char *comands[COMMAND_SIZE];
	int i=0;
	char *parte = strtok(comando,"|");
	while(parte!=NULL){
		comands[i] = parte;
		parte=strtok(NULL,"|");
		i++;
	}

	int p[i][2];
	int inat[i-1][2];
	int status;
	comands[i]='\0';
	int j;


	char readfrompipe[MAX_SIZE_BUFFER];
	int bytes,flog,totalbytes;

	int flag=0;

	int o,pid;
	
	signal(SIGUSR1,sigusr1_inatividade_handler);			//defenição do handler para o tempo maximo de inatividade (recebe um SIGUSR1 de um dos filhos)
	signal(SIGUSR2,sigusr2_terminar_handler);				//defenição do handler para o trminar um processo (recebe um SIGUSR2 da main)


	signal(SIGALRM,sigalrm_execucao_handler);				//defenição do handler para o tempo maximo de execucao
	if(te>0)alarm(te);										//alarme para o tempo maximo de execucao
	
	
	for(j=0;j<=i;j++){
		if(j==0){
			if(pipe(p[j])==-1){
                perror("Pipe");
                return -1;
            }

    		switch((pidsexecucao[j]=fork())){
               	case -1:
                    perror("Fork");
                 	return -1;
                case 0:
             		close(p[j][0]);
                   	dup2(p[j][1],1);
                  	close(p[j][1]);

                   	_exit(mysystem(comands[j]));
                            
    			default:
    				close(p[j][1]);
            }
		}
		
		else if(j==i){
			switch((pidsaver=fork())){
                case -1:
                    perror("Fork");
                    return -1;
                case 0:
                
					totalbytes=0;
                	while((bytes = read(inat[j-1][0],readfrompipe,MAX_SIZE_BUFFER))>0){
                		if(!flag){
	                   		if((flog=open("log",O_CREAT | O_WRONLY , 0666))==-1){
								perror("open log");
								return -1;
							}
							slogidx.index=lseek(flog,0,SEEK_END);
							flag=1;
						}
						

						if(write(flog,readfrompipe,bytes)==-1){
							perror("write log");
						}
						totalbytes+=bytes;
                	}

                	close(inat[j-1][0]);
                	close(flog);
					slogidx.size=totalbytes;
					if((flogidx=open("log.idx",O_CREAT | O_WRONLY,  0666))==-1){
						perror("open log.idx");
						return -1;
					}
					
					lseek(flogidx,poslogidx,SEEK_SET);
					
					if(write(flogidx,&slogidx,sizeof(struct logidx))==-1){
						perror("write log.idx");
					}
					

                	close(flogidx);
					                
                   	_exit(0);

                default:
                	break;
            }
		}
		
		
		else{
			if(pipe(p[j])==-1){
                perror("Pipe");
                return -1;
            }

            switch((pidsexecucao[j]=fork())){
                case -1:
                    perror("Fork");
                    return -1;
                case 0:
                    dup2(inat[j-1][0],0);
                    close(inat[j-1][0]);


                    close(p[j][0]);
                    dup2(p[j][1],1);
                    close(p[j][1]);

                    _exit(mysystem(comands[j]));

                default:
                    close(p[j][1]);
            }
		}

		//criação de pipes intermédios para a verificação de inatividade
        if(j<i){
            if(pipe(inat[j])==-1){
                perror("Pipe de inatividade");
                return -1;
            }

            switch((pid=fork())){
                case -1:
                    perror("Fork");
                    return -1;
                case 0:
                    signal(SIGALRM,sigalrm_pipeinativo_handler);						//handler do alarme de inatividade
                    
                    alarm(ti);															//alarme de inatividade
                    while((o=read(p[j][0],readfrompipe,MAX_SIZE_BUFFER))>0){
                        alarm(0);
                        write(inat[j][1],readfrompipe,o);
                    }

                    close(p[j][0]);
                    
                    _exit(0);

                default:
                    close(inat[j][1]);
            }
        }

	}
	
	for(j=0;j<2*i;j++){
		wait(&status);
	}
	alarm(0);	
	wait(&status);

	return 0;
}

void sigchld_handler(int sig){
	int p=wait(NULL);
	int enc=0;
	for (int i = start_tarefas; i < tarefa_index && !enc; i++){
		if(pids[i]==p){
			pids[i]=-1;
			enc=1;		
		}
	}
}
void sig_not_handler(int sig){}			//retirar o handler do SIGCHLD aos filhos


void getidx(){
	struct logidx slogidx;
	int flogidx;
	if((flogidx=open("log.idx",O_CREAT | O_RDONLY,  0666))==-1){
		perror("open log.idx");
	}
	
	while((read(flogidx,&slogidx,sizeof(struct logidx)))>0){
		if(slogidx.tarefa>tarefa_index)tarefa_index=slogidx.tarefa;
	}

	tarefa_index++;

}


void writetofifo2(char *string){
	int fd2=open(FIFO2,O_WRONLY);
	if(fd2==-1){
		perror("Open fifo2");
	}
	if(write(fd2,string,strlen(string))==-1){
		perror("write");
	}
	close(fd2);
}

void writehistorico(){
	struct logidx slogidx;
	int flogidx;
	if((flogidx=open("log.idx",O_CREAT | O_RDONLY,  0666))==-1){
		perror("open log.idx");
	}

	int fd2=open(FIFO2,O_WRONLY);
	if(fd2==-1){
		perror("Open fifo2");
	}
	char string[1060];
	while((read(flogidx,&slogidx,sizeof(struct logidx)))>0){
		if(slogidx.estado[0]!='u'){
			sprintf(string,"#%d, %s: %s\n",slogidx.tarefa,slogidx.estado,slogidx.valorc);
			if(write(fd2,string,strlen(string))==-1){
				perror("write");
			}
		}
	}
	close(fd2);
	close(flogidx);
}

void writeoutput(int n){
	struct logidx slogidx;
	int flogidx,enc=0;
	if((flogidx=open("log.idx",O_CREAT | O_RDONLY,  0666))==-1){
		perror("open log.idx for writeoutput");
	}
	while(!enc && (read(flogidx,&slogidx,sizeof(struct logidx)))>0){
		if(slogidx.tarefa==n) enc=1;
	}
	close(flogidx);

	int fd2=open(FIFO2,O_WRONLY);
	if(fd2==-1){
		perror("Open fifo2");
	}

	if(enc && slogidx.estado[0]=='c'){
		int fd3=open("log",O_CREAT | O_RDONLY , 0666);
		if(fd3==-1){
			perror("Open log  for writeoutput");
		}
		int o;
		char buffer[100];
		lseek(fd3,slogidx.index,SEEK_SET);
		while((o=read(fd3,buffer,100))>0 && slogidx.size>0){
			if(write(fd2,buffer,(o>slogidx.size?slogidx.size:o))==-1){
				perror("write output de uma tarefa");
			}
			slogidx.size-=o;
		}
		close(fd3);

	}
	close(fd2);
}

void writeemexecucao(){
	int fd2=open(FIFO2,O_WRONLY);
	if(fd2==-1){
		perror("Open fifo2");
	}

	char string[100];
	for (int i = 0; i < tarefa_index; ++i){
		if(pids[i]>0){
			sprintf(string,"#%d: %s\n",i,comandos[i]);
			if(write(fd2,string,strlen(string))==-1){
				perror("write tarefas em execucao");
			}
		}
	}

	close(fd2);
}

int main (int argc, char* argv[]){
	int op=1;
	int bytes_read;
	int fd;

	signal(SIGINT,sigint_handler);
	signal(SIGCHLD,sigchld_handler);
	
	
	getidx();
	sizepids=2*tarefa_index;
	start_tarefas=tarefa_index;
	pids=realloc(pids,sizepids*sizeof(int));
	comandos=realloc(comandos,sizepids*sizeof(char*));

	if(mkfifo(FIFO1,0666)==-1){
		perror("mkfifo");
	}

	if(mkfifo(FIFO2,0666)==-1){
		perror("mkfifo2");
	}
	
	char string[100];


	while(op){
		if((fd=open(FIFO1,O_RDONLY))==-1){
			perror("open fifo");
			return -1;
		}
		//kill(getpid(),SIGCHLD);
		struct estrutura s;
		while((bytes_read = read(fd,&s,sizeof(struct estrutura)))>0){
			switch(s.comando){
				case 1://tempo-inatividade n
					if(s.valori>=0)ti=s.valori;
					break;
				case 2://tempo-execucao n
					if(s.valori>=0)te=s.valori;
					break;
				case 3://execcutar 'comando'
					if (tarefa_index>=sizepids){
						sizepids=2*tarefa_index;
						pids=realloc(pids,sizepids*sizeof(int*));
						comandos=realloc(comandos,sizepids*sizeof(char*));
					}

					switch((pids[tarefa_index]=fork())){
						case -1:
							perror("fork");
							return -1;
						case 0:
							signal(SIGCHLD,sig_not_handler);
							signal(SIGINT,sig_not_handler);

							executar(s.valorc);
							_exit(0);
							break;
						default:
							break;
					}
					
					comandos[tarefa_index]=strdup(s.valorc);
					sprintf(string,"nova tarefa #%d\n",tarefa_index);
					writetofifo2(string);
					tarefa_index++;	
					break;
				case 4://listar
					writeemexecucao();
					break;
				case 5://terminar
					if(s.valori>0 && pids[s.valori]>0){
						kill(pids[s.valori],SIGUSR2);
					}
					break;//historico
				case 6:
					writehistorico();
					break;
				case 8://output 8
					writeoutput(s.valori);
					break;
				default:
					perror("opcao invalida\n");
					break;
			}
			
		}


		if(bytes_read==0);
		else perror("read");

		close(fd);
	}	


	unlink(FIFO1);
	unlink(FIFO2);

	return 0;
}