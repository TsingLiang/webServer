#ifndef EVENT_H
#define EVENT_H

struct Epoll;

typedef void (*readCallback)(struct Epoll* epoll, void* arg);
typedef void (*writeCallback)(struct Epoll* epoll, void *arg);

typedef struct Event
{
    int fd;
    short events;
    readCallback readCb;
    writeCallback writeCb;
}Event;

#define setFd(event, sockfd) \
do                                      \
{                                       \
    (event)->fd = sockfd;               \
}while(0)                               

#define getFd(event) ((event)->fd)

#define enableRead(event)   \
do                                              \
{                                               \
    (event)->events |= EPOLLIN;                   \
}while(0)                                   

#define enableWrite(event)   \
do                                              \
{                                               \
    (event)->events |= EPOLLOUT;                   \
}while(0)                                   

#define disableRead(event)   \
do                                              \
{                                               \
    (event)->events &= ~EPOLLIN;                   \
}while(0)                                   

#define disableWrite(event)   \
do                                              \
{                                               \
    (event)->events &= ~EPOLLOUT;                   \
}while(0)                                   

#define getEvents(event) ((event)->events)

#define setReadCallback(event, cb)  \
do                                                          \
{                                                           \
    (event)->readCb = cb;                               \
}while(0)   

#define getReadCallback(event) ((event)->readCallback)

#define setWriteCallback(event, cb)   \
do                                                              \
{                                                               \
    (event)->writeCb = cb;                                 \
}while(0)                                   

#define getWriteCallback(event) ((event)->writeCallback)

#endif
