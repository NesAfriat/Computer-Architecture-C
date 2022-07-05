all: exec

exec: task2b.c 
	gcc -g -m32 -c -o virusDetector.o task2b.c
	gcc -g -m32  virusDetector.o -o virusDetector
	rm virusDetector.o

.PHONY: clean
clean:
	rm -rf ./*.o virusDetector
