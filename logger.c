#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


#define LOG "log2"
#define LOGIDX "log.idx2"

#define SIZEVALORC 64
struct logger{
	int tarefa_index;
	int modo;				//0->criar,1->adicionar log,2->alterar estado
	char valor[504];		//504 pois mais dois inteiros da 512, tamanho de escrita que assegura que a escrita no pipe é de um unico write
};
struct logger s;

struct logidx{
	int tarefa;	
	char estado[10];
	char valorc[SIZEVALORC];
	int index[100];			//consegue gravar 100*504 bytes de output para cada tarefa
	int size[100];			
};


void criarlogidx(){
	int flogidx,enc=0;
	if((flogidx=open(LOGIDX,O_CREAT | O_RDWR,  0666))==-1){
		perror("open log.idx");
	}

	struct logidx slogidx;
	slogidx.tarefa=s.tarefa_index;
	strcpy(slogidx.estado,"erro");
	strcpy(slogidx.valorc,s.valor);
	slogidx.size[0]=0;

	if(write(flogidx,&slogidx,sizeof(slogidx))==-1){
		perror("write");
	}

	close(flogidx);
}

int adicionarlog(){
	int flogidx,enc=0;
	if((flogidx=open(LOGIDX,O_CREAT |O_APPEND| O_RDWR,  0666))==-1){
		perror("open log.idx");
	}
	
	struct logidx slogidx;
	while(!enc && (read(flogidx,&slogidx,sizeof(struct logidx)))>0){
		if(slogidx.tarefa==s.tarefa_index) enc=1;
	}
	if(enc)	lseek(flogidx,-1*sizeof(struct logidx),SEEK_CUR);

	int i=0,flog;
	while(slogidx.size[i]!=0)i++;

	if((flog=open(LOG,O_CREAT | O_WRONLY , 0666))==-1){
		perror("open log");
		return -1;
	}
	slogidx.index[i]=lseek(flog,0,SEEK_END);
	if(write(flog,s.valor,strlen(s.valor))==-1){
		perror("write");
	}
	slogidx.size[i]=strlen(s.valor);
	slogidx.size[i+1]=strlen(s.valor);
	close(flog);	

	if(write(flogidx,&slogidx,sizeof(slogidx))==-1){
		perror("write");
	}

	close(flogidx);
	return 0;
}

int main (int argc, char* argv[]){
	int o;
	while((o=read(0,&s,sizeof(struct logger)))>0){
		printf("[LOGGER] %d:modo %d->%s\n", s.tarefa_index,s.modo,s.valor);
		switch(s.modo){
			case 0:
				printf("criar em logidx\n");
				criarlogidx();
				break;
			case 1:
				printf("adicionar no log\n");
				printf("%d\n",adicionarlog());
				break;
			case 2:
				printf("alterar estado\n");
				break;
			default:
				printf("erro\n");
				break;
		}
	}
	return 0;
}
