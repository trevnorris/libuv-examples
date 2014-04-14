#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "uv.h"

typedef struct {
  uv_async_t async;
  uv_mutex_t mutex;
  uv_thread_t thread;
} thread_comm_t;


void async_cb(uv_async_t* async, int status) {
  printf("async_cb\n");
  uv_close((uv_handle_t*) async, NULL);
}


void thread_cb(void* arg) {
  thread_comm_t* comm = (thread_comm_t*) arg;

  printf("pre async send\n");
  uv_async_send(&comm->async);
  printf("post async send\n");
}


int main() {
  thread_comm_t comm;
  int r;

  memset(&comm, 0, sizeof(comm));

  r = uv_mutex_init(&comm.mutex);
  assert(r == 0);

  r = uv_async_init(uv_default_loop(), &comm.async, async_cb);
  assert(r == 0);
  comm.async.data = (void*) &comm;

  r = uv_thread_create(&comm.thread, thread_cb, (void*) &comm);
  assert(r == 0);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  return 0;
}
