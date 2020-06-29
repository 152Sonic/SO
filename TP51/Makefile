all:argusd argus
projeto:clean argusd argus

argusd:argusd.c argus.h
	gcc -Wall -g -o argusd argusd.c

argus:argus.c argus.h
	gcc -Wall -g -o argus argus.c

clean:
	rm -f argusd argus log log.idx fifo fifo2
