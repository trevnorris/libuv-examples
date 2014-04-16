#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "uv.h"

static uv_mutex_t mutex;
static uv_thread_t thread;

static void thread_cb(void* arg) {
  printf("thread_cb\n");
  uv_mutex_lock(&mutex);
  printf("thread mutex\n");
  uv_mutex_unlock(&mutex);
}

int main() {
  assert(0 == uv_mutex_init(&mutex));
  assert(0 == uv_thread_create(&thread, thread_cb, NULL));

  uv_mutex_lock(&mutex);
  printf("main mutex start\n");
  sleep(1);
  printf("main mutex end\n");
  uv_mutex_unlock(&mutex);

  uv_thread_join(&thread);
  uv_mutex_destroy(&mutex);

  return 0;
}
