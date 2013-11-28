#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
const int flag = 1;

static void *
cancelme(void *bb_in)
{
  pthread_mutex_lock(&mutex);

  /* Install a cleanup handler */
  pthread_cleanup_push(pthread_mutex_unlock, &mutex);

  while (flag) {
    pthread_cond_wait(&cv, &mutex);
  }

  /* Uninstall cleanup handler */
  pthread_cleanup_pop(1);

  return (NULL);
}

int
main()
{
  pthread_t t;

  pthread_create(&t, NULL, cancelme, NULL);

  sleep(1);

  pthread_cancel(t);

  printf("Waiting for thread to finish...\n");
  pthread_join(t, NULL);
  printf("Thread finished...\n");

  printf("Waiting to acquire lock...\n");
  pthread_mutex_lock(&mutex);
  printf("Lock acquired\n");

  return (0);
}
