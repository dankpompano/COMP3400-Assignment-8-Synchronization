/*
 * COMP 3400: Template project driver
 *
 * Name: 
 */

#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mutex.h"
#include "queue.h"

int cmdline (int, char**, bool*, bool*);

void
usage (void)
{
  printf ("Usage: synch option [option...]\n");
  printf (" At least one argument must be passed\n");
  printf (" Options are:\n");
  printf ("  -m      Execute the runner() thread to demonstrate semaphores\n");
  printf ("  -q      Run the fifo_queue() to demonstrate a queue of size 1\n");
}

int
main (int argc, char **argv)
{
  bool run_mutex = false;
  bool run_queue = false;
  if (cmdline (argc, argv, &run_mutex, &run_queue) < 0)
    {
      usage ();
      return EXIT_FAILURE;
    }

  // TODO: Initialize this lock and pass a reference to it to run() and
  // fifo_queue() to use for mutual exclusion.
  pthread_mutex_t lock;
  struct pcarg queue;
  pthread_mutex_init (&lock, NULL);
  run(&lock);
  fifo_queue(&lock, queue.size);


  // MIN requirements: Use semaphores for mutual exclusion
  if (run_mutex)
    {
      printf ("Running mutual exclusion test\n");
      int64_t shared = run (&lock);
      printf ("Value of shared: %" PRId64 "\n", shared);
    }

  // FULL requirements: Use locks and semaphores to demonstrate
  // producer/consumer pattern with a queue of size 10. Note that the
  // unit tests use different queue sizes, but integration tests will
  // just use 10.
  if (run_queue)
    {
      printf ("Running a simulation of a synchronized FIFO queue\n");
      fifo_queue (&lock, 10);
      printf ("\n");
    }

  // TODO: Now destroy the lock since it is no longer needed.
  pthread_mutex_destroy (&lock);
  pthread_exit (NULL);
}

/*****************************************************************************
 ****************** DO NOT MODIFY FUNCTIONS IN THIS SECTION ******************
 *****************************************************************************/

int
cmdline (int argc, char **argv, bool *mutex, bool *queue)
{
  int option;

  while ((option = getopt (argc, argv, "mqh")) != -1)
    {
      switch (option)
        {
        case 'm': *mutex = true;
                  break;
        case 'q': *queue = true;
                  break;
        case 'h': return -1;
                  break;
        default:  return -1;
        }
    }

  if (!mutex && !queue)
    {
      printf ("You must pass at least one argument\n");
      usage ();
      return -1;
    }
  return 0;
}
