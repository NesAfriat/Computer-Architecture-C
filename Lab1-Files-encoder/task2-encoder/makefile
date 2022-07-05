all: exec

exec: encoder.c 
	gcc -g -m32 -c -o encoder.o encoder.c
	gcc -g -m32  encoder.o -o encoder
	rm encoder.o

.PHONY: clean
clean:
	rm -rf ./*.o encoder
