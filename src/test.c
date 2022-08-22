#include <pthread.h>
#include <stdio.h>

#include "cqueue.h"

Cqueue* cq = NULL;

void* thread_push(void* arg) {
  pthread_detach(pthread_self());
  cq_push(cq, pthread_self());
  return NULL;
}

void* thread_pop(void* arg) {
  pthread_detach(pthread_self());
  cq_pop(cq);
  return NULL;
}

int cqueue_test() {
  cq = cq_init(100);

  const int n = 1001001;
  for (int i = 0; i < n; ++i) {
    if (i & 1) {
      pthread_t tid;
      pthread_create(&tid, 0, thread_push, NULL);
    } else {
      pthread_t tid;
      pthread_create(&tid, 0, thread_pop, NULL);
    }
  }
  if (cq->size != 0) {
    printf("FAILED: size[%d]\n", cq->size);
  } else {
    printf("OK: size[%d]\n", cq->size);
  }
  cq_finalize(cq);
  return cq->size == 0;
}
