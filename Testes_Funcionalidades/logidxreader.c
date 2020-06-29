#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
struct logidx{
	int tarefa;	
	char estado[20];
	char valorc[SIZE];
	int index;
	int size;
};

int main(int argc, char *argv[]){
	int o,o2;
	int d1 = open("../log.idx",O_RDONLY);
	if(d1<0){
        perror("log.idx");
        return -1;
    }
	
    int lo=1;
	int d2 = open("../log",O_RDONLY);
	if(d2<0){
        //perror("log");
        lo=0;
    }
    
    int index,size;
    int sread;
    char buffer[1024];
    struct logidx l;
    while((o=read(d1,&l,sizeof(struct logidx)))){
    	printf("%d -> %s,%s,%d,%d\n",l.tarefa,l.estado,l.valorc,(index=l.index),(size=l.size));

    	printf("\n");
    	
        if(lo){
        	lseek(d2,index,SEEK_SET);
    					
    		
            while((sread = read(d2,buffer,1024))>0 && size>0){
    			if(write(1,buffer,(sread>size?size:sread))==-1){
    				perror("write");
    			}
    			size-=sread;
            }
        }

        printf("---------------------------------\n\n");

    }

}
