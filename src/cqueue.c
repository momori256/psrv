#include "cqueue.h"

#include <stdlib.h>

Cqueue* cq_init(int capacity) {
  Cqueue* cq = (Cqueue*)malloc(sizeof(Cqueue));
  cq->fds = (int*)malloc(sizeof(int) * capacity);
  cq->capacity = capacity;
  cq->size = 0;
  cq->head = 0;
  cq->tail = 0;

  {
    const int shared_between_processes = 0;
    const int init_val = 1;
    sem_init(&cq->mutex, shared_between_processes, init_val);
  }
  return cq;
}

void cq_finalize(Cqueue* const cq) {
  sem_destroy(&cq->mutex);
  free(cq->fds);
  free(cq);
}

void cq_push(Cqueue* const cq, int fd) {
  while (cq->size >= cq->capacity) {
  }
  sem_wait(&cq->mutex);
  while (cq->size >= cq->capacity) {
    sem_post(&cq->mutex);
    return;
  }

  cq->fds[cq->tail] = fd;
  cq->tail = (cq->tail + 1) % cq->capacity;
  ++cq->size;

  sem_post(&cq->mutex);
}

int cq_pop(Cqueue* const cq) {
  while (cq->size <= 0) {
  }
  sem_wait(&cq->mutex);
  if (cq->size <= 0) {
    sem_post(&cq->mutex);
    return -1;
  }

  int top = cq->fds[cq->head];
  cq->head = (cq->head + 1) % cq->capacity;
  --cq->size;

  sem_post(&cq->mutex);
  return top;
}
