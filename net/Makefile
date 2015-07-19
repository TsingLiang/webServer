LIBS = -lpthread
OBJS = SocketTest.o Socket.o
SocketTest: ${OBJS}
	gcc -o SocketTest ${OBJS}
EpollTest: Epoll.o EpollTest.o Socket.o
	gcc -o EpollTest Epoll.o EpollTest.o Socket.o 
clean:
	rm -f Socket ${OBJS}

