#include <semaphore.h>

// Concurrent Queue.
typedef struct Cqueue {
  int* fds;
  int capacity;
  int size;
  int head;
  int tail;
  sem_t mutex;
} Cqueue;

Cqueue* cq_init(int capacity);

void cq_finalize(Cqueue* const cq);

void cq_push(Cqueue* const cq, int fd);

int cq_pop(Cqueue* const cq);
