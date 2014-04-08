#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "uv.h"

#define QUANTITY 1000000
#define SIZE 1

typedef struct {
  int id;
  int size;
  int cntr;
  uv_async_t async;
} async_req_t;

static int q_cntr = 0;

static void after(uv_work_t* req, int status);


static void async_send_cb(uv_async_t* handle, int status) {
  /*async_req_t* async_req = (async_req_t*) handle->data;*/
  /*printf("async_send_cb-%i : %i\n", async_req->id, async_req->cntr);*/
}


static void work_cb(uv_work_t* req) {
  async_req_t* async_req = (async_req_t*) req->data;
  int size = async_req->size;
  /*printf("work_cb-%i\n", async_req->id);*/

  for (int i = 0; i < size; i++) {
    async_req->cntr = i;
    uv_async_send(&async_req->async);
  }
}


static void setup_req(uv_work_t** req,
                      async_req_t** async_req,
                      int id,
                      int size) {
  int r;

  *req = (uv_work_t*) malloc(sizeof(**req));
  assert(*req != NULL);

  *async_req = (async_req_t*) malloc(sizeof(**async_req));
  assert(*async_req != NULL);

  (*async_req)->id = id;
  (*async_req)->size = size;
  (*async_req)->cntr = 0;

  r = uv_async_init(uv_default_loop(), &((*async_req)->async), async_send_cb);
  assert(r == 0);

  (*async_req)->async.data = (void*) (*async_req);
  (*req)->data = (void*) (*async_req);

  r = uv_queue_work(uv_default_loop(), *req, work_cb, after);
  assert(r == 0);
}


static void after(uv_work_t* req, int status) {
  /*printf("after-%i\n", ((async_req_t*)(req->data))->id);*/
  uv_close((uv_handle_t*)(&((async_req_t*) req->data)->async), NULL);
  free(req->data);
  free(req);

  if (++q_cntr < QUANTITY) {
    uv_work_t* req;
    async_req_t* async;
    setup_req(&req, &async, q_cntr, SIZE);
  }
}


int main() {
  for (int i = 0; i < 3; i++) {
    q_cntr++;
    uv_work_t* req;
    async_req_t* async;
    setup_req(&req, &async, q_cntr, SIZE);
  }


  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
