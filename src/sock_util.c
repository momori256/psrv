#include "sock_util.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static const int BACKLOG = 1024;

static int create_sockfd_client(const addrinfo *const addrhd);
static int create_sockfd_server(const addrinfo *const addrhd);
static int set_reuseaddr(int sockfd);

int create_connectfd(const char *const hostname, const char *const port) {
  const addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM,
      .ai_flags = AI_PASSIVE,
  };
  addrinfo *addrhd;
  const int res = getaddrinfo(hostname, port, &hints, &addrhd);
  if (res != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
    exit(1);
  }

  const int sockfd = create_sockfd_client(addrhd);
  if (sockfd == -1) {
    fprintf(stderr, "create_sockfd: %s\n", strerror(errno));
    exit(1);
  }

  freeaddrinfo(addrhd);
  return sockfd;
}

int create_listenfd(const char *const hostname, const char *const port) {
  const addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM,
      .ai_flags = AI_PASSIVE,
  };
  addrinfo *addrhd;
  const int res = getaddrinfo(hostname, port, &hints, &addrhd);
  if (res != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
    exit(1);
  }

  const int sockfd = create_sockfd_server(addrhd);
  if (sockfd == -1) {
    fprintf(stderr, "create_sockfd: %s\n", strerror(errno));
    exit(1);
  }
  freeaddrinfo(addrhd);

  if (listen(sockfd, BACKLOG) == -1) {
    fprintf(stderr, "listen: %s\n", strerror(errno));
    exit(1);
  }

  return sockfd;
}

static int set_reuseaddr(int sockfd) {
  int reuse = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
                 sizeof(reuse)) == 0) {
    return 0;
  }
  fprintf(stderr, "setsockopt: %s\n", strerror(errno));
  exit(1);
}

static int create_sockfd_client(const addrinfo *const addrhd) {
  for (const addrinfo *ap = addrhd; ap != NULL; ap = ap->ai_next) {
    int sockfd = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
    if (sockfd == -1) {
      continue;
    }
    if (connect(sockfd, ap->ai_addr, ap->ai_addrlen) == 0) {
      return sockfd;
    }
  }
  return -1;
}

static int create_sockfd_server(const addrinfo *const addrhd) {
  for (const addrinfo *p = addrhd; p != NULL; p = p->ai_next) {
    int sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      continue;
    }
    set_reuseaddr(sockfd);
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
      return sockfd;
    }
    close(sockfd);
  }
  return -1;
}
