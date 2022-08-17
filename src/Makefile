FLAGS = -Wall -c -g

psrv: main.o server.o client.o sock_util.o
	cc -Wall -g -o psrv main.o server.o client.o sock_util.o

main.o: main.c server.o client.o
	cc $(FLAGS) -o main.o main.c

server.o: server.c sock_util.o
	cc $(FLAGS) -o server.o server.c

client.o: client.c sock_util.o
	cc $(FLAGS) -o client.o client.c

sock_util.o: sock_util.c sock_util.h
	cc $(FLAGS) -o sock_util.o sock_util.c

clean:
	-rm *.o psrv
