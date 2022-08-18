FLAGS = -Wall -c -g -std=c99
MACROS = -D_POSIX_C_SOURCE=200112L

psrv: main.o server.o client.o sock_util.o cmd.o
	cc -Wall -g $(MACROS) -lpthread -o psrv main.o server.o client.o sock_util.o cmd.o

main.o: main.c server.o client.o
	cc $(FLAGS) $(MACROS) -o main.o main.c

server.o: server.c sock_util.o cmd.o
	cc $(FLAGS) $(MACROS) -o server.o server.c

client.o: client.c sock_util.o
	cc $(FLAGS) $(MACROS) -o client.o client.c

sock_util.o: sock_util.c sock_util.h
	cc $(FLAGS) $(MACROS) -o sock_util.o sock_util.c

cmd.o: cmd.c cmd.h
	cc $(FLAGS) $(MACROS) -o cmd.o cmd.c

clean:
	-rm *.o psrv
