#include <stdio.h>
#include <assert.h>

#include "uv.h"

static uv_thread_t thread;


static void thread_cb(void* arg) {
  printf("hello thread!\n");
}


int main() {
  int r = uv_thread_create(&thread, thread_cb, NULL);
  assert(r == 0);

  /* pause execution of this thread until the spawned thread has had
   * time to finish execution. */
  uv_thread_join(&thread);

  return 0;
}
