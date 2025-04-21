#ifndef __mutex_h__
#define __mutex_h__

#include <pthread.h>
#include <stdint.h>

void * runner (void *);
int64_t run (pthread_mutex_t *);

#endif
