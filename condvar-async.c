#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "uv.h"

typedef struct {
  uv_mutex_t* mutex;
  uv_async_t* async;
  uv_cond_t* cond;
  uv_thread_t* thread;
  uv_loop_t* loop;
  int alive;
} cross_comm_t;


void comm_thread_cb(void* arg) {
  cross_comm_t* comm = (cross_comm_t*) arg;

  while (comm->alive > 0) {
    uv_mutex_lock(comm->mutex);
    uv_cond_wait(comm->cond, comm->mutex);
    uv_mutex_unlock(comm->mutex);

    uv_async_send(comm->async);
  }
}


void comm_async_cb(uv_async_t* async, int status) {
  printf("comm_async_cb\n");
  /*cross_comm_t* comm = (cross_comm_t*) async->data;*/
}


void cross_comm_init(cross_comm_t* comm,
                     uv_loop_t* loop,
                     void (*async_cb)(uv_async_t*, int),
                     void (*thread_cb)(void*)) {
  int r;

  comm = (cross_comm_t*) malloc(sizeof(*comm));
  assert(comm != NULL);
  comm->mutex = (uv_mutex_t*) malloc(sizeof(*comm->mutex));
  assert(comm->mutex != NULL);
  comm->async = (uv_async_t*) malloc(sizeof(*comm->async));
  assert(comm->async != NULL);
  comm->cond = (uv_cond_t*) malloc(sizeof(*comm->cond));
  assert(comm->cond != NULL);
  comm->thread = (uv_thread_t*) malloc(sizeof(*comm->thread));
  assert(comm->thread != NULL);

  comm->loop = loop;

  r = uv_mutex_init(comm->mutex);
  assert(r == 0);

  r = uv_async_init(uv_default_loop(), comm->async, (*async_cb));
  assert(r == 0);
  comm->async->data = (void*) comm;

  r = uv_cond_init(comm->cond);

  r = uv_thread_create(comm->thread, (*thread_cb), (void*) comm);
  assert(r == 0);

  comm->alive = 1;
}


void conn_close_cb(uv_handle_t* arg) {
  cross_comm_t* comm = (cross_comm_t*) ((uv_async_t*) arg)->data;

  uv_mutex_destroy(comm->mutex);
  uv_cond_destroy(comm->cond);

  free(comm->mutex);
  free(comm->async);
  free(comm->cond);
  free(comm->thread);
  free(comm);
}


void cross_comm_destroy(cross_comm_t* comm) {
  uv_thread_join(comm->thread);
  uv_close((uv_handle_t*) comm->async, conn_close_cb);
}


int main() {
  cross_comm_t* comm = NULL;
  int r;

  cross_comm_init(comm, uv_default_loop(), comm_async_cb, comm_thread_cb);

  r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  cross_comm_destroy(comm);

  return r;
}
