#include <stdlib.h>
#include <stdio.h>
#include <assert.h>  // assert
#include <unistd.h>  // sleep
#include <string.h>  // memset

#include "uv.h"

typedef struct {
  uv_mutex_t mutex;
  uv_cond_t cond;
  int delay;
  int use_broadcast;
  volatile int posted;
} worker_config;


static void worker(void* arg) {
  printf("worker\n");

  worker_config* wc = (worker_config*) arg;

  uv_mutex_lock(&wc->mutex);
  uv_cond_signal(&wc->cond);
  uv_mutex_unlock(&wc->mutex);
}


int main() {
  uv_thread_t thread;
  worker_config wc;

  memset(&wc, 0, sizeof(wc));

  uv_cond_init(&wc.cond);
  uv_mutex_init(&wc.mutex);
  uv_thread_create(&thread, worker, &wc);

  uv_mutex_lock(&wc.mutex);
  uv_cond_wait(&wc.cond, &wc.mutex);
  uv_mutex_unlock(&wc.mutex);

  uv_thread_join(&thread);
  uv_mutex_destroy(&wc.mutex);
  uv_cond_destroy(&wc.cond);

  return 0;
}
