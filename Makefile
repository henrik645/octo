octo : octo.o
	gcc -o octo octo.o -Wall

octo.o : octo.c
	gcc -c octo.c -Wall
