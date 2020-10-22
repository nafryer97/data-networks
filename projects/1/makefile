server : server.o
	gcc -o server server.o

client : client.o
	gcc -o client client.o

tests : tests.o
	gcc -o tests tests.o

server.o : server.h server.c
	gcc -c server.c

client.o : client.c client.h
	gcc -c client.c

tests.o : tests.c server.h
	gcc -c tests.c

clean :
	rm client.o server.o tests.o 
