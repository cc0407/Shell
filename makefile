all: main

main: main.c
	gcc -Wall -pedantic -std=c99 main.c -o myShell

clean:
	rm myShell