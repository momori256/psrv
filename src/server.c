#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "sock_util.h"

typedef struct epoll_event epoll_event;
typedef struct sockaddr_in sockaddr_in;

typedef struct ReadCtx {
  int epfd;
  int clifd;
} ReadCtx;

const int BUF_SIZE = 4096;

static void epoll_add_infd(int epfd, int fd);
static void epoll_del_fd(int epfd, int fd);
static void accept_conn(int epfd, int listenfd);
static int read_clifd(int epfd, int clifd, char *buf);
static void *thread(void *arg);

int echo() {
  const int epfd = epoll_create1(0);
  if (epfd == -1) {
    fprintf(stderr, "epoll_create1: %s\n", strerror(errno));
    exit(1);
  }
  const int listenfd = create_listenfd("localhost", "8080");
  epoll_add_infd(epfd, listenfd);

  const int MAX_EVENTS = 10;
  epoll_event events[MAX_EVENTS];

  while (1) {
    const int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      fprintf(stderr, "epoll_wait: %s\n", strerror(errno));
      exit(1);
    }

    for (int i = 0; i < nfds; ++i) {
      if (events[i].data.fd == listenfd) {
        accept_conn(epfd, listenfd);
        continue;
      }

      const int clifd = events[i].data.fd;
      epoll_del_fd(epfd, clifd);
      pthread_t tid;
      ReadCtx *const ctx = (ReadCtx *)malloc(sizeof(ReadCtx));
      {
        ctx->epfd = epfd;
        ctx->clifd = clifd;
      }
      const int err = pthread_create(&tid, NULL, thread, (void *)ctx);
      if (err) {
        fprintf(stderr, "pthread_create: %d\n", err);
        exit(1);
      }
    }
  }
  return 0;
}

static void epoll_add_infd(int epfd, int fd) {
  epoll_event event = {
      .events = EPOLLIN,
      .data.fd = fd,
  };
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1) {
    fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
    exit(1);
  }
}

static void epoll_del_fd(int epfd, int fd) {
  if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
    fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
    exit(1);
  }
}

static void *thread(void *arg) {
  const pthread_t tid = pthread_self();
  fprintf(stderr, "thread begin: tid[%lu]\n", tid);
  pthread_detach(tid);

  const ReadCtx *ctx = (ReadCtx *)arg;
  const int epfd = ctx->epfd;
  const int clifd = ctx->clifd;

  char *buf = (char *)malloc(BUF_SIZE);
  const int closed = read_clifd(epfd, clifd, buf);

  free(arg);
  free(buf);
  if (!closed) {
    epoll_add_infd(epfd, clifd);
  }
  fprintf(stderr, "thread end: tid[%lu]\n", tid);
  pthread_exit(NULL);
}

static int read_clifd(int epfd, int clifd, char *buf) {
  int read_size = 0;
  while ((read_size = read(clifd, buf, sizeof(buf))) > 0) {
    if (buf[read_size - 1] == '\n') {
      buf[read_size - 1] = '\0';
    } else {
      buf[read_size] = '\0';
    }
    fprintf(stdout, "recved msg: %s\n", buf);

    if (strncmp(buf, "quit", sizeof("quit") - 1) == 0) {
      // epoll automatically removes clifd from interest sets.
      if (close(clifd) == -1) {
        fprintf(stderr, "close: %s\n", strerror(errno));
        exit(1);
      }
      fprintf(stdout, "quit connection: fd[%d]\n", clifd);
      return 1;
    }

    if (strncmp(buf, "end", sizeof("end") - 1) == 0) {
      break;
    }
  }

  if (read_size == -1) {
    fprintf(stderr, "recv: %s\n", strerror(errno));
    exit(1);
  }
  if (read_size == 0) {  // closed from client.
    // epoll automatically removes clifd from interest sets.
    if (close(clifd) == -1) {
      fprintf(stderr, "close: %s\n", strerror(errno));
      exit(1);
    }
    fprintf(stdout, "close connection: fd[%d]\n", clifd);
    return 1;
  }
  return 0;
}

static void accept_conn(int epfd, int listenfd) {
  sockaddr_in cliaddr;
  socklen_t cliaddr_len = sizeof(cliaddr);
  const int clifd = accept(listenfd, (sockaddr *)&cliaddr, &cliaddr_len);
  if (clifd == -1) {
    fprintf(stderr, "accept: %s\n", strerror(errno));
    exit(1);
  }
  fprintf(stdout, "accpet connection from addr[%s], port[%d]\n",
          inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
  epoll_add_infd(epfd, clifd);
}
