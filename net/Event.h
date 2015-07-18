#ifndef EVENT_H
#define EVENT_H

typedef void (*readCallback)(void* arg);
typedef void (*writeCallback)(void *arg);

typedef _event
{
    int sockfd;
    short events;
    readCallback readCb;
    writeCallBack writeCb;
}Event;

#define setFd(Event* event, int sockfd) \
do                                      \
{                                       \
    (event)->fd = sockfd;               \
}while(0)                               

#define getFd(Event* event) ((event)->fd)

#define setEvents(Event* event, short events)   \
do                                              \
{                                               \
    (event)->events = events;                   \
}while(0)                                   

#define getEvents(Event* event) ((event)->events)

#define setReadCallback(Event* event, readCallback readCb)  \
do                                                          \
{                                                           \
    (event)->readCb = readCb;                               \
}while(0)   

#define getReadCallback(Event* event) ((event)->readCallback)

#define setWriteCallback(Event* event, writeCallback writeCb)   \
do                                                              \
{                                                               \
    (event)->writeCb = writeCb;                                 \
}while(0)                                   

#define getWriteCallback(Event* event) ((event)->writeCallback)

#endif
