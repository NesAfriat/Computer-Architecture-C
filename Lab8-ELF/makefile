all: myELF1 myELF2 myELF3

myELF3: task3.o
		gcc -g -Wall -m32 -o myELF3 task3.o 
		rm -rf ./*.o

task3.o: task3.c
		gcc -g -Wall -m32 -c -o task3.o task3.c

myELF2: task2.o
		gcc -g -Wall -m32 -o myELF2 task2.o 
		rm -rf ./*.o

task2.o: task2.c
		gcc -g -Wall -m32 -c -o task2.o task2.c

myELF1: task1.o
		gcc -g -Wall -m32 -o myELF1 task1.o 
		rm -rf ./*.o

task1.o: task1.c
		gcc -g -Wall -m32 -c -o task1.o task1.c


.PHONY: clean

clean:
		rm -rf myELF1 myELF2 myELF3