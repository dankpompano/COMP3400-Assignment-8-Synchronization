#include <assert.h>
#include <check.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "../mutex.h"
#include "../queue.h"

#include "private.h"

double time_diff (struct timeval start, struct timeval end)
{
  double ending = end.tv_sec + (end.tv_usec * 0.000001);
  double starting = start.tv_sec + (start.tv_usec * 0.000001);
  return ending - starting;
}

START_TEST (MIN_mutex_run_two_threads)
{
  printf ("======================================\n");
  printf ("PUBLIC\n");
  printf ("  MIN mutex run two threads\n");
  printf ("  Call runner in two threads and ensure result is 0\n");

  pthread_mutex_t lock;
  pthread_mutex_init (&lock, NULL);
  int64_t shared = run (&lock);
  pthread_mutex_destroy (&lock);
  ck_assert_int_eq (shared, 0);
  printf ("\n");
}
END_TEST

START_TEST (MIN_mutex_run_two_threads_time)
{
  printf ("======================================\n");
  printf ("PRIVATE\n");
  printf ("  MIN mutex runner two threads time\n");
  printf ("  Call runner in two threads and measure time\n");
  printf ("  Multithreaded should take more than 3 times and\n");
  printf ("    less than 6 times the unithreaded time.\n");

  int64_t num = 0;
  int64_t *shared = &num;

  struct timeval start, end;
  gettimeofday (&start, NULL);
  for (int j = 0; j < 1000000; j++)
    {
      for (int i = 0; i < 100; i++)
        {
          *shared += 1;
          *shared -= 1;
        }
    }
  gettimeofday (&end, NULL);
  double unidiff = time_diff (start, end);

  pthread_mutex_t lock;
  pthread_mutex_init (&lock, NULL);
  gettimeofday (&start, NULL);
  num = run (&lock);
  gettimeofday (&end, NULL);
  double multidiff = time_diff (start, end);

  printf ("\n  Unithreaded time: %f\n  Multithreaded time: %f\n\n",
          unidiff, multidiff);

  ck_assert_int_eq (num, 0);
  ck_assert (unidiff * 6 > multidiff);
  ck_assert (unidiff * 3 < multidiff);
  printf ("\n");

}
END_TEST

/*
   1 - Create a queue of size 1 and put things in one at a time, pausing
       for 50 ms in between.
   2 - Create a queue of size 8, fill it, then start the worker.
   3 - Create a queue of size 15, start the listener, then pull one item
       at a time.
   4 - Just use fifo_queue for size 10, 20, and 30.
*/
START_TEST (FULL_singleton)
{
  printf ("======================================\n");
  printf ("PUBLIC\n");
  printf ("  FULL worker\n");
  printf ("  Create a queue of size 1\n");

  char *a = NULL;
  char *r = NULL;
  prodcons_t *pc = build_args (1, 1, &a, &r);
  pthread_t workers[5];

  for (int i = 0; i < 5; i++)
    pthread_create (&workers[i], NULL, worker, (void *)pc);

  for (int i = 0; i < 25; i++)
    {
      pc->queue[0] = i;
      usleep (50000);
      sem_post (pc->ready);
    }

  for (int i = 0; i < 5; i++)
    pthread_join (workers[i], NULL);

  ck_assert_int_eq (pc->next_in, 0);
  ck_assert_int_eq (pc->next_out, 0);
  destroy_args (pc, a, r);
  free (pc);
  printf ("\n");
}
END_TEST

START_TEST (FULL_prefilled)
{
  printf ("======================================\n");
  printf ("PUBLIC\n");
  printf ("  FULL prefilled\n");
  printf ("  Create and pre-fill a queue of size 5\n");

  char *a = NULL;
  char *r = NULL;
  prodcons_t *pc = build_args (8, 2, &a, &r);
  pthread_t workers[5];

  for (int i = 0; i < 8; i++)
    {
      pc->queue[i] = i;
      sem_post (pc->ready);
    }

  for (int i = 0; i < 5; i++)
    pthread_create (&workers[i], NULL, worker, (void *)pc);

  for (int i = 0; i < 8; i++)
    sem_wait (pc->available);

  for (int i = 8; i < 25; i++)
    {
      sem_wait (pc->available);
      pc->queue[i % 8] = i;
      sem_post (pc->ready);
    }

  for (int i = 0; i < 5; i++)
    pthread_join (workers[i], NULL);

  ck_assert_int_eq (pc->next_in, 0);
  ck_assert_int_eq (pc->next_out, 1);
  destroy_args (pc, a, r);
  free (pc);
  printf ("\n");
}
END_TEST

START_TEST (FULL_let_prefill)
{
  printf ("======================================\n");
  printf ("PUBLIC\n");
  printf ("  FULL let prefill\n");
  printf ("  Create a queue of size 15 and let listener fill it\n");

  char *a = NULL;
  char *r = NULL;
  prodcons_t *pc = build_args (15, 3, &a, &r);
  pthread_t listener;

  pthread_create (&listener, NULL, listen, (void *)pc);

  printf ("  Sleeping for 2 seconds, allowing buffer to fill\n");
  sleep (2);

  printf ("  Now downing the semaphore to make sure\n");
  for (int i = 0; i < 15; i++)
    sem_wait (pc->ready);

  printf ("  Now checking the values and allowing listen to proceed\n");
  for (int i = 0; i < 15; i++)
    {
      int value = pc->queue[i];
      ck_assert_int_eq (value, i);
      sem_post (pc->available);
    }

  for (int i = 0; i < 10; i++)
    {
      sem_wait (pc->ready);
      int value = pc->queue[i];
      ck_assert_int_eq (value, i + 15);
      sem_post (pc->available);
    }

  pthread_join (listener, NULL);

  ck_assert_int_eq (pc->next_in, 10);
  ck_assert_int_eq (pc->next_out, 0);
  destroy_args (pc, a, r);
  free (pc);
  printf ("\n");
}
END_TEST

START_TEST (FULL_queue_10)
{
  printf ("======================================\n");
  printf ("PUBLIC\n");
  printf ("  FULL queue 10\n");
  printf ("  Create a queue of size 10 and let it run\n");

  pthread_mutex_t lock;
  pthread_mutex_init (&lock, NULL);
  fifo_queue (&lock, 10);
  pthread_mutex_destroy (&lock);
  ck_assert (true);
}
END_TEST

START_TEST (FULL_queue_20)
{
  printf ("======================================\n");
  printf ("PUBLIC\n");
  printf ("  FULL queue 20\n");
  printf ("  Create a queue of size 20 and let it run\n");

  pthread_mutex_t lock;
  pthread_mutex_init (&lock, NULL);
  fifo_queue (&lock, 20);
  pthread_mutex_destroy (&lock);
  ck_assert (true);
}
END_TEST

START_TEST (FULL_queue_30)
{
  printf ("======================================\n");
  printf ("PUBLIC\n");
  printf ("  FULL queue 30\n");
  printf ("  Create a queue of size 30 and let it run\n");

  pthread_mutex_t lock;
  pthread_mutex_init (&lock, NULL);
  fifo_queue (&lock, 30);
  pthread_mutex_destroy (&lock);
  ck_assert (true);
}
END_TEST

void public_tests (Suite *s)
{
  TCase *tc_public = tcase_create ("Public");
  tcase_set_timeout (tc_public, 5.0);
  tcase_add_test (tc_public, MIN_mutex_run_two_threads);
  tcase_add_test (tc_public, MIN_mutex_run_two_threads_time);
  tcase_add_test (tc_public, FULL_singleton);
  tcase_add_test (tc_public, FULL_prefilled);
  tcase_add_test (tc_public, FULL_queue_10);
  tcase_add_test (tc_public, FULL_queue_20);
  tcase_add_test (tc_public, FULL_queue_30);
  suite_add_tcase (s, tc_public);
}
