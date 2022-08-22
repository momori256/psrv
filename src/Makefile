FLAGS = -Wall -c -g -std=c99 -ldl -lpthread
MACROS = -D_POSIX_C_SOURCE=200112L

psrv: main.o server.o server_pool.o client.o sock_util.o cmd.o cqueue.o test.o
	cc -Wall -g $(MACROS) -lpthread -o psrv main.o server.o server_pool.o client.o sock_util.o cmd.o cqueue.o test.o

main.o: main.c server.o client.o
	cc $(FLAGS) $(MACROS) -o main.o main.c

server.o: server.c sock_util.o cmd.o
	cc $(FLAGS) $(MACROS) -o server.o server.c

server_pool.o: server_pool.c sock_util.o cmd.o
	cc $(FLAGS) $(MACROS) -o server_pool.o server_pool.c

client.o: client.c sock_util.o
	cc $(FLAGS) $(MACROS) -o client.o client.c

sock_util.o: sock_util.c sock_util.h
	cc $(FLAGS) $(MACROS) -o sock_util.o sock_util.c

cmd.o: cmd.c cmd.h
	cc $(FLAGS) $(MACROS) -o cmd.o cmd.c

cqueue.o: cqueue.c cqueue.h
	cc $(FLAGS) $(MACROS) -o cqueue.o cqueue.c

test.o: test.c cqueue.o
	cc $(FLAGS) $(MACROS) -o test.o test.c

clean:
	-rm *.o psrv
