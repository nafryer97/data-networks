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

submission : server.c server.h client.c client.h
	cp client.c fryerP1Client.c
	cp server.c fryerP1Server.c
	cp client.h fryerP1Client.h
	cp server.c fryerP1Server.h

clean :
	rm client.o server.o tests.o server client tests fryerP1* 
