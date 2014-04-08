#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "uv.h"

#define RUNS 1e4

typedef struct {
  uv_async_t parent;
  uv_async_t worker;
  uv_loop_t worker_loop;
  uv_thread_t worker_thread;
} thread_comm;

int cntr = 0;


static void parent_cb(uv_async_t* handle, int status) {
  thread_comm* comm = (thread_comm*) handle->data;
  uv_async_send(&(comm->worker));

  if (++cntr < RUNS)
    return;

  uv_thread_join(&(comm->worker_thread));
  uv_close((uv_handle_t*) &(comm->parent), NULL);
}


static void worker_close(uv_handle_t* handle) {
  thread_comm* comm = (thread_comm*) handle;
  uv_stop(&(comm->worker_loop));
  uv_loop_close(&(comm->worker_loop));
}


static void worker_cb(uv_async_t* handle, int status) {
  thread_comm* comm = (thread_comm*) handle->data;
  if (cntr < RUNS)
    uv_async_send(&(comm->parent));
  else
    uv_close((uv_handle_t*) &(comm->worker), worker_close);
}


static void thread_entry(void* arg) {
  int r;

  thread_comm* comm = (thread_comm*) arg;

  r = uv_loop_init(&(comm->worker_loop));
  assert(r == 0);

  r = uv_async_init(&(comm->worker_loop), &(comm->worker), worker_cb);
  assert(r == 0);
  comm->worker.data = (void*) comm;

  uv_async_send(&(comm->parent));

  uv_run(&(comm->worker_loop), UV_RUN_DEFAULT);
}


int main() {
  thread_comm comm;
  uint64_t time;
  int r;

  r = uv_async_init(uv_default_loop(), &(comm.parent), parent_cb);
  assert(r == 0);
  comm.parent.data = (void*) &comm;

  r = uv_thread_create(&(comm.worker_thread), thread_entry, (void*) &comm);
  assert(r == 0);

  time = uv_hrtime();

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  uv_loop_close(uv_default_loop());

  time = uv_hrtime() - time;
  printf("test complete: %i\n", cntr);
  printf("time: %f\n", time / 1e9);
  printf("back-n-forth/sec: %f\n", ((cntr * 1.0) / (time / 1e9)) * 1.0);
  printf("ns/op: %f\n", (time * 1.0) / (cntr * 1.0));

  return 0;
}
