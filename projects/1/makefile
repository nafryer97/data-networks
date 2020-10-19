server : server.o
	gcc -o server server.o

client : client.o
	gcc -o client client.o

server.o : server.h server.c
	gcc -c server.c

client.o : client.c client.h
	gcc -c client.c

clean :
	rm client.o server.o 
