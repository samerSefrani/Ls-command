all: myls

myls: myls.c
	gcc -g -Wall -o myls myls.c -lpthread

valgrind:
	valgrind --leak-check=full ./myls 5000 127.0.0.1 5001

clean:
	$(RM) myls