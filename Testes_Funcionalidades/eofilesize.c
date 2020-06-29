#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
	int o,o2;
	int d1 = open("cenas.txt",O_CREAT | O_RDWR, 0640);
	if(d1<0){
        perror("cenas");
        return -1;
    }
    char output[100];

	printf("%d\n",(o=lseek(d1,0,SEEK_END)));

	write(d1,"ola",3);

	printf("%d -> %d\n",o,(o2=lseek(d1,0,SEEK_END)));
	

	//////////////////////////////////////

	
	o=105;
	o2=108;

	lseek(d1,o,SEEK_SET);

	int size=read(d1,output,o2-o);
	write(1,output,size);

}
