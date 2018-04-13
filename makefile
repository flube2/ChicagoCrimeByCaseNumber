build:
	clear
	gcc -Wall -std=c99 -pedantic -g main.c avl.c mymem.c -o crimes

run:
	clear
	./crimes
