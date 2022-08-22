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
#include <time.h>
#include <unistd.h>

#include "cmd.h"
#include "cqueue.h"
#include "sock_util.h"

typedef struct epoll_event epoll_event;
typedef struct sockaddr_in sockaddr_in;

typedef struct ReadCtx {
  int epfd;
  int clifd;
} ReadCtx;

static const int BUF_SIZE = 4096;

static void epoll_add_infd(int epfd, int fd);
static void epoll_del_fd(int epfd, int fd);
static void accept_conn(int epfd, int listenfd);
static int read_msg(int clifd, char *const buf);
static void *thread(void *arg);

static int epfd = -1;
static Cqueue *cq = NULL;

int echo_pool() {
  epfd = epoll_create1(0);
  if (epfd == -1) {
    fprintf(stderr, "epoll_create1: %s\n", strerror(errno));
    exit(1);
  }
  const int listenfd = create_listenfd("localhost", "8080");
  epoll_add_infd(epfd, listenfd);

  const int MAX_EVENTS = 100;
  epoll_event events[MAX_EVENTS];

  const int CAPACITY = 100;
  cq = cq_init(CAPACITY);

  const int NTHREAD = 4;
  for (int i = 0; i < NTHREAD; ++i) {
    pthread_t tid;
    pthread_create(&tid, NULL, thread, NULL);
  }

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
      printf("push clifd[%d].\n", clifd);
      cq_push(cq, clifd);
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

  char *const msg = (char *)malloc(BUF_SIZE * sizeof(char));
  while (1) {
    const int clifd = cq_pop(cq);
    if (clifd < 0) {
      continue;
    }
    const int closed = read_msg(clifd, msg);

    if (!closed) {
      char buf[100];
      const int res = call_cmd(msg, buf);
      char send[120];
      sprintf(send, "[%d]%s from [%llu]\r\n", res, buf,
              (unsigned long long)tid);
      write(clifd, send, strlen(send));

      epoll_add_infd(epfd, clifd);
    }
    memset(msg, 0, BUF_SIZE * sizeof(char));

    printf("proceeded by thread[%llu]\n", (unsigned long long)tid);
  }

  free(msg);
  fprintf(stderr, "thread end: tid[%lu]\n", tid);

  pthread_exit(NULL);
}

static int endswith(const char *const s, const char *const end) {
  const size_t elen = strlen(end);
  const size_t slen = strlen(s);
  if (slen < elen) {
    return 0;
  }
  for (size_t i = 0; i < elen; ++i) {
    if (s[i + slen - elen] != end[i]) {
      return 0;
    }
  }
  return 1;
}

static int read_msg(int clifd, char *const buf) {
  int ntotal = 0;
  int nread = 0;
  while ((nread = read(clifd, buf + ntotal, sizeof(buf))) > 0) {
    ntotal += nread;
    if (endswith(buf, "\r\n")) {
      buf[ntotal] = '\0';
      break;
    }
  }

  // printf("read_msg [%s].\n", buf);
  if (nread == -1) {
    fprintf(stderr, "recv: %s\n", strerror(errno));
    exit(1);
  }
  if (nread == 0) {  // closed from client.
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
