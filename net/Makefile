LIBS = -lpthread
OBJS = SocketTest.o Socket.o
CLAGS = "-g"
SocketTest: ${OBJS}
	gcc -o SocketTest ${OBJS}
EpollTest: Epoll.o EpollTest.o Socket.o
	gcc -o EpollTest Epoll.o EpollTest.o Socket.o 
clean:
	rm -f Socket ${OBJS} Epoll.o EpollTest.o

Socket.o: Socket.c Socket.h
	gcc -g -c Socket.c
SocketTest.o:SocketTest.c SocketTest.h Socket.h
	gcc -g -c SocketTest.c
Epoll.o: Epoll.c Epoll.h Event.h
	gcc -g -c Epoll.c
EpollTest.o: EpollTest.c Epoll.h Event.h
	gcc -g -c EpollTest.c
