all:eofilesize logidxreader2 logidxreader alarmes inatividade ftemporario

eofilesize:eofilesize.c
	gcc -Wall -g -o eofilesize eofilesize.c

logidxreader2:logidxreader2.c
	gcc -Wall -g -o logidxreader2 logidxreader2.c

logidxreader:logidxreader.c
	gcc -Wall -g -o logidxreader logidxreader.c

alarmes:alarmes.c
	gcc -Wall -g -o alarmes alarmes.c

inatividade:tempoinatividade.c
	gcc -Wall -g -o inatividade tempoinatividade.c

ftemporario:ficheirotemporario.c
	gcc -Wall -g -o ftemporario ficheirotemporario.c


clean:
	rm -f eofilesize logidxreader logidxreader2 alarmes inatividade ftemporario
