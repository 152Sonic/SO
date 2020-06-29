#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include "argus.h"


void writemsg(char* string){
	write(1,string,strlen(string));
}
void writeerror(char* string){
	write(2,string,strlen(string));
}


int validateop(char* string){
	int res=-1;

	if(strcmp(string,"tempo-inatividade")==0)res=1;
	else if(strcmp(string,"tempo-execucao")==0)res=2;
	else if(strcmp(string,"executar")==0)res=3;
	else if(strcmp(string,"listar")==0)res=4;
	else if(strcmp(string,"terminar")==0)res=5;
	else if(strcmp(string,"historico")==0)res=6;
	else if(strcmp(string,"ajuda")==0)res=7;
	else if(strcmp(string,"output")==0)res=8;

	return res;
}



int alldigits(char* string){
    int i,flag=1;
    for( i=0;i<strlen(string)&&flag;i++){
        flag=isdigit(string[i]);
    }
    return flag;
}

int toint(char* string){
	int res=-1;
    if(alldigits(string)) res= atoi(string);
    return res;
}


int writetofifo(struct estrutura s){
	int fd=open(FIFO1,O_WRONLY);
	if(fd==-1){
		perror("Open fifo");
		return -1;
	}

	if(write(fd,&s,sizeof(struct estrutura))==-1){
		perror("write");
	}

	close(fd);
	return 0;
}

int readfromfifo2(){
	int fd2=open(FIFO2,O_RDONLY);
	if(fd2==-1){
		perror("Open fifo");
		return -1;
	}
	int o;
	char string[100];
	while((o=read(fd2,&string,100))>0){
		write(1,string,o);
	}

	close(fd2);
	return 0;

}


int ti=-1;		//tempo-inatividade (!=0 se tiver sido usada a opcao)
int te=-1;		//tempo-execucao
char *cmd=NULL;	//posicao no argv do comando
int li=-1;		//se e para listar
int idt=-1;		//id do processo que é para terminar
int hist=-1;	//se é para listar historico
int aj=-1;		//se e para listar a ajuda
int out=-1;		//id da instrucao que se quer o output

int getflags(int argc, char* argv[]){
	int i=1;
	//ver se tem flags
	while(i<argc){
		if(argv[i][0]!='-')return -1;
		if(argv[i][0]=='-' && argv[i][1]!='\0'&& argv[i][2]=='\0'){
			
			//ver flag escolhidas
			switch (argv[i][1]){
				case 'i':
					if(argv[i+1]){
						ti=toint(argv[i+1]);
					}else return -1;
					if(ti<0)return -1;
					i+=2;
					break;
				case 'm':
					if(argv[i+1]){
						te=toint(argv[i+1]);
					}else return -1;
					if(te<0)return -1;
					i+=2;
					break;
				case 'e':
					if(argv[i+1]){
						cmd=argv[i+1];
					}else return -1;
					i+=2;
					break;
				case 'l':
					li=1;
					i+=1;
					break;
				case 't':
					if(argv[i+1]){
						idt=toint(argv[i+1]);
					}else return -1;
					if(idt<0) return -1;
					i+=2;
					break;
				case 'r':
					hist=1;
					i+=1;
					break;
				case 'h':
					aj=1;
					i+=1;
					break;
				case 'o':
					if(argv[i+1]){
						out=toint(argv[i+1]);
					}else return -1;
					i+=2;
					break;
				default:
					return -1;
			}
		}else{
			return -1;
		}
	}
	return 0;
}

int main (int argc, char* argv[]){
	char *stringajuda = strdup("tempo-inactividade segs\ntempo-execucao segs\nexecutar \'p1 | p2 ... | pn\'\nlistar\nterminar n\nhistorico\najuda\noutput n\n");

	if(argc < 2){//linha de comandos
		char buffer[MAX_SIZE_BUFFER]; //char representa 1byte e nao por ser um ficheiro de texto
		int n;
		char inputline[10]="argus$ ";
		int op=1;
		char* p = NULL;
		char* aux = NULL;


		int v;

		struct estrutura s;


		write(1,inputline,strlen(inputline));
		while((n = read(0,buffer,MAX_SIZE_BUFFER))>0){

			buffer[n]='\0';
			free(aux);
			aux=strdup(buffer);

			p=strtok(buffer," \n");
			op=validateop(p);
			s.comando=op;
			s.valori=-1;
			strcpy(s.valorc,"\0");

			switch(op){
				case 1://tempi-inatividade n
					p=strtok(NULL," \n");

					if(p && (v=toint(p))>=0 && !strtok(NULL," \n")){
						s.valori = v;
						writetofifo(s);
					}
					else writeerror("comando invalido\n");
					break;

				case 2://tempo-execucao n
					p=strtok(NULL," \n");
					if(p && (v=toint(p))>=0 && !strtok(NULL," \n")){
						s.valori = v;						
						writetofifo(s);
					}
					else writeerror("comando invalido\n");
					break;

				case 3://executar 'comando'
					//printf("1fuck %s you\n",buffer);
					p=strtok(aux,"\'\n");
					p=strtok(NULL,"\'\n");
					if(p && !strtok(NULL,"\'\n")){
						strcpy(s.valorc,p);					
						writetofifo(s);
						readfromfifo2();
					}
					else writeerror("comando invalido\n");
					break;

				case 4://listar
					if(!strtok(NULL," \n")){					
						writetofifo(s);
						readfromfifo2();
					}
					else writeerror("comando invalido\n");
					break;

				case 5://terminar n
					p=strtok(NULL," \n");
					if(p && (v=toint(p))>=0 && !strtok(NULL," \n")){
						s.valori = v;						
						writetofifo(s);
					}
					else writeerror("comando invalido\n");
					break;

				case 6://historico
					if(!strtok(NULL," \n")){					
						writetofifo(s);
						readfromfifo2();
					}
					else writeerror("comando invalido\n");
					break;

				case 7://ajuda
					writemsg(stringajuda);
					break;

				case 8://output n
					p=strtok(NULL," \n");

					if(p && (v=toint(p))>=0 && !strtok(NULL," \n")){
						s.valori = v;
						writetofifo(s);
						readfromfifo2();
					}
					else writeerror("comando invalido\n");
					break;

				default:
					writeerror("opcao invalida\n");
					break;
			}

			
			//printf("converting to sommin\n");
			//write(1,buffer,n);

			write(1,inputline,strlen(inputline));
		}

		write(1,"^D\n",3);

	}else{//comando ja inserido
		int res=0;
		if(getflags(argc,argv)==0){
			struct estrutura s;
			if(aj==1){
				writemsg(stringajuda);
				return 0;
			}
			if(li==1){
				s.comando=4;
				res= res || writetofifo(s);
				readfromfifo2();		
				return 0;
			}
			if(hist==1){
				s.comando=6;
				res= res || writetofifo(s);
				readfromfifo2();	
				return 0;
			}
			if(idt>-1){
				s.comando=5;
				s.valori=idt;
				res= res || writetofifo(s);
				return 0;
			}
			if(ti>-1){
				s.comando=1;
				s.valori=ti;
				res= res || writetofifo(s);
			}
			if(te>-1){
				s.comando=2;
				s.valori=te;
				res= res || writetofifo(s);				
			}
			if(cmd){
				s.comando=3;
				strcpy(s.valorc,cmd);
				res= res || writetofifo(s);
				readfromfifo2();				
			}
			if(out>-1){
				//writemsg("funcionalidade output indisponivel\n");
				s.comando=8;
				s.valori=out;
				res= res || writetofifo(s);				
				readfromfifo2();
			}
		}else{
			writeerror("Erro na instrucao\n");
		}
	}

	return 0;
}