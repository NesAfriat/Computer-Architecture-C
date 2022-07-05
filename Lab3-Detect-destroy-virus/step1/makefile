all: exec

exec: task1c.c 
	gcc -g -m32 -c -o virusDetector.o task1c.c
	gcc -g -m32  virusDetector.o -o virusDetector
	rm virusDetector.o

.PHONY: clean
clean:
	rm -rf ./*.o virusDetector
