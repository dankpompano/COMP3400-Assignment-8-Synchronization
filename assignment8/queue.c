#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pcarg {
    pthread_mutex_t lock;
    sem_t *ready;
    sem_t *available;
    int *queue;
    size_t size;
    size_t next_in;
    size_t next_out;
} prodcons_t;

/* Each worker thread will pretend to be processing 5 incoming socket
   connections. The arguments passed should be a pointer to an instance of
   the struct declared above. The semaphores are used to ensure the queue
   is accessed safely, while the lock should be used to atomically execute
   the removal and printing of the item.
 */
void *
worker (void * _args)
{
  prodcons_t *args = (prodcons_t *) _args;

  // TODO: Fix this for-loop, using synchronization primitives to (safely)
  // retrieve an item from the circular queue and print it out.
  for (int i = 0; i < 5; i++)
    {
      int socket = args->queue[args->next_out++];
      if (args->next_out >= args->size)
        args->next_out = 0;
      printf ("Processing socket %d\n", socket);
    }

  pthread_exit (NULL);
}

/* Thread that simulates listening for incoming network connections (like
   those from a web server). The thread will generate 25 requests and place
   them (safely) into the request queue. Note that the capacity of the queue
   might be smaller than 25, requiring synchronization.
 */
void *
listen (void * _args)
{
  prodcons_t *args = (prodcons_t *) _args;

  // TODO: Fix this for-loop using synchronization primitives to (safely) add
  // items into the queue for printing. Note that, since there is only one
  // listener thread inserting items (only one thread modifying the next_in
  // variable), there is no need to access the lock.
  for (int i = 0; i < 25; i++)
    {
      args->queue[args->next_in++] = i;
      if (args->next_in >= args->size)
        args->next_in = 0;
    }

  pthread_exit (NULL);
}

void
fifo_queue (pthread_mutex_t *lock, size_t queue_size)
{
  pthread_t listener;
  pthread_t workers[5];
  
  // TODO: Initialize the semaphores based on the definition of the
  // Producer-Consumer problem (see Section 8.3). The available count
  // should be set to the queue size (because there are that many spots
  // available initially), and the ready should be 0 (because there are
  // no requests initially ready for processing).

  sem_t item_ready;
  sem_t space_available;  

  // TODO: Setup a SINGLE instance of args to store the lock and pointers
  // to the two semaphores.
  prodcons_t args;

  // Create a queue of the specified size
  args.queue = calloc (queue_size, sizeof (int));
  args.size = queue_size;
  args.next_in = 0;
  args.next_out = 0;  

  // TODO: Pass this SINGLE instance by reference to 5 worker threads and
  // the listener thread. Use the same for-loop structure as shown here, but
  // change the last parameter of the pthread_create() calls to point to the
  // struct instance.
  for (int i = 0; i < 5; i++)
    pthread_create (&workers[i], NULL, worker, NULL);
  pthread_create (&listener, NULL, listen, NULL);

  for (int i = 0; i < 5; i++)
    pthread_join (workers[i], NULL);
  pthread_join (listener, NULL);

  // TODO: Clean up the semaphores and the queue before returning.
}
