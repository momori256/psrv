#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "sock_util.h"

int keep_sending_msg() {
  const int connfd = create_connectfd("localhost", "8080");
  if (connfd <= 0) {
    exit(1);
  }

  const int BUF_SIZE = sizeof("cnt: -0123456789");
  char buf[BUF_SIZE];
  int cnt = 0;
  while (1) {
    snprintf(buf, BUF_SIZE, "cnt: %d", cnt++);
    const int sentSize = send(connfd, buf, strlen(buf), 0);
    if (sentSize < 0) {
      fprintf(stderr, "send: %s\n", strerror(errno));
      exit(1);
    }
    sleep(1);
  }
}
