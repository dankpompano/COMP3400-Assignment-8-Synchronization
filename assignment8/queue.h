#ifndef __queue_h__
#define __queue_h__

#include <pthread.h>

extern int *queue;
extern size_t size;

void * worker (void *);
void * listen (void *);
void fifo_queue (pthread_mutex_t *, size_t);

#endif
