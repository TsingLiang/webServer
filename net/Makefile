LIBS = -lpthread
OBJS = SocketTest.o Socket.o
#CFLAGS = "-g"
#SocketTest: ${OBJS}
#	gcc -o SocketTest ${OBJS}
#EpollTest: Epoll.o EpollTest.o Socket.o
#	gcc -o EpollTest Epoll.o EpollTest.o Socket.o 
clean:
	rm -f EventLoopTest Epoll.o EpollTest.o Socket.o Event.o EventLoop.o

EventLoopTest: EventLoopTest.o Epoll.o Socket.o SignalEvent.o Event.o EventLoop.o
	gcc -o EventLoopTest EventLoopTest.o Epoll.o Socket.o SignalEvent.o Event.o EventLoop.o

Socket.o: Socket.c Socket.h
	gcc -g -c Socket.c
#SocketTest.o:SocketTest.c SocketTest.h Socket.h
#	gcc -g -c SocketTest.c
Epoll.o: Epoll.c Epoll.h Event.h
	gcc -g -c Epoll.c
Event.o: Event.c Event.h
	gcc -g -c Event.c
#EpollTest.o: EpollTest.c Epoll.h Event.h
#	gcc -g -c EpollTest.c
EventLoop.o: EventLoop.c EventLoop.h Event.h Epoll.h SignalEvent.h
	gcc -g -c EventLoop.c
SignalEvent.o: SignalEvent.c SignalEvent.h Event.h Epoll.h
	gcc -g -c SignalEvent.c
EventLoopTest.o: EventLoopTest.c EventLoop.h Event.h
	gcc -g -c EventLoopTest.c
