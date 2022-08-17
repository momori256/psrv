#pragma once

typedef struct addrinfo addrinfo;
typedef struct sockaddr sockaddr;

// create a socket fd connecting to hostname:port host.
int create_connectfd(const char *const hostname, const char *const port);

// create a socket fd for accepting.
int create_listenfd(const char *const hostname, const char *const port);
