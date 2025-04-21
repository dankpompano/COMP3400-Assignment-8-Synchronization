#ifndef __PRIVATE_H__
#define __PRIVATE_H__

typedef struct pcarg {
    pthread_mutex_t lock;
    sem_t *ready;
    sem_t *available;
    int *queue;
    size_t size;
    size_t next_in;
    size_t next_out;
} prodcons_t;

prodcons_t *build_args (size_t, int, char **, char **);
void destroy_args (prodcons_t *, char *, char *);

//sem_t * build_semaphore (int, int, char **);
//void exec_runner_threads (int);
//void exec_cook_thread (int);

#endif
