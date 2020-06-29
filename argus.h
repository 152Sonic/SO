#ifndef _ARGUS_
#define _ARGUS_


#define MAX_SIZE_BUFFER 1024			//tamanho do buffer para leitura dos pipes anonimos


#define COMMAND_SIZE 1024		
struct estrutura{
	int comando;				//inteiro referente ao comando a executar
	int valori;				//informação int  a transmitir (ex.: terminar)
	char valorc[COMMAND_SIZE];		//informação string a transmitir (executar)
};

#define FIFO1 "fifo"
#define FIFO2 "fifo2"

#endif
