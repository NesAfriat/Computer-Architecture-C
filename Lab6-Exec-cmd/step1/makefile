all: mypipeline

mypipeline: task1.o
		gcc -g -Wall -m32 -o mypipeline task1.o
		rm -rf ./*.o


task1.o: task1.c
		gcc -g -Wall -m32 -c -o task1.o task1.c


.PHONY: clean

clean:
		rm -rf mypipeline
