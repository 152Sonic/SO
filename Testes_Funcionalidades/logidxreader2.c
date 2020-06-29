#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZEVALORC 64
struct logidx{
    int tarefa; 
    char estado[10];
    char valorc[SIZEVALORC];
    int index[100];
    int size[100];
};

int main(int argc, char *argv[]){
	int o,o2;
	int d1 = open("../log.idx2",O_RDONLY);
	if(d1<0){
        perror("log.idx2");
        return -1;
    }
	
    int lo=0;
	int d2 = open("../log2",O_RDONLY);
	if(d2<0){
        //perror("log");
        lo=0;
    }
    
    int index,size;
    int sread;
    char buffer[1024];
    struct logidx l;
    while((o=read(d1,&l,sizeof(struct logidx)))){
    	printf("%d -> %s,%s,%d,%d\n",l.tarefa,l.estado,l.valorc,(index=l.index[0]),(size=l.size[0]));

    	printf("\n");
    	
        if(lo){
    		int i=0;	
    		while(size!=0){
        	   lseek(d2,index,SEEK_SET);
                while((sread = read(d2,buffer,size))>0 && size>0){
                    if(write(1,buffer,(sread>size?size:sread))==-1){
                       perror("write");
                    }
                    size-=sread;
                }
                size=l.size[++i];
                index=l.index[i];
            }
            
        }

        printf("---------------------------------\n\n");

    }

}
