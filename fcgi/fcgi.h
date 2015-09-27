#ifndef FCGI_H
#define FCGI_H

#define FCGI_LISTEN_FD 0
#define NEW_FCGI_LISTEN_FD 3

int fcgi_accept(int argc, char* argv[]);

#endif
