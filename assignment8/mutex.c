#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Use the struct here for passing arguments to the runner() threads.
// All instances of this struct should point to the same pthread_mutex_t
// and shared variables.
typedef struct args {
    pthread_mutex_t *lock;
    int64_t *shared;
} arg_t;

/* Function to run in concurrent threads. The argument passed should be a
   pointer to a pthread_mutex_t to be used for mutual exclusion. Within the
   nested for-loop structure, use the mutex to protect the the increments
   and decrements of the shared variable. To get the timing right for the
   unit tests, experiment with placing the lock/unlock calls in different
   places (e.g., around both for-loops, inside the outer one, inside the
   inner one, etc.) */
void *
runner (void *arg)
{
  // TODO: Replace this local variable with a pointer retrieved from the
  // struct passed as an argument.
  int64_t *shared = &arg->shared;

  // TODO: Use the lock in various places here to protect the increments and
  // decrements of the shared variable. Keep the for-loop structure as written,
  // with the outer loop executing 100,000 times and the inner loop executing
  // 1,000 times.
  for (int j = 0; j < 1000000; j++)
    {
      for (int i = 0; i < 100; i++)
        {
          *shared += 1;
          *shared -= 1;
        }
    }

  pthread_exit (NULL);
}

/* Simple fork-join routine that creates two threads running the runner()
   function above. Pass the lock and a pointer to the shared variable to
   both threads. When both threads complete, return the shared variable,
   which should once again have a value of 0.
 */
int64_t
run (pthread_mutex_t *lock)
{
  int64_t shared = 0;
  struct args arguments;
  arguments.lock = &lock;
  arguments.shared = shared;

  // TODO: Create and join two threads. Both threads should have an argument
  // containing a pointer to the lock and to the shared variable above.
  pthread_t thread1, thread2;
  pthread_create (&thread1, NULL, runner, (void *)&arguments);
  pthread_create (&thread2, NULL, runner, (void *)&arguments);
  
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  
  return shared;
}
