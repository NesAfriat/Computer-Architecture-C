all: hexeditplus digit_cnt

hexeditplus: task1.o
		gcc -g -Wall -m32 -o hexeditplus task1.o 
		

task1.o: task1.c
		gcc -g -Wall -m32 -c -o task1.o task1.c

digit_cnt: task4.c 
	gcc -g -m32 -fno-pie -c -o task4.o task4.c
	gcc -g -m32 -fno-pie task4.o -o digit_cnt
	rm -rf ./*.o

.PHONY: clean

clean:
		rm -rf hexeditplus digit_cnt