#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "uv.h"

#define RUNS 1e5

uv_async_t* async_h;
int cntr = 0;


static void run_test() {
  /* uv_async_send always returns 0. */
  uv_async_send(async_h);
}


static void async_cb(uv_async_t* handle, int status) {
  if (++cntr >= RUNS)
    uv_close((uv_handle_t*) async_h, NULL);
  else
    run_test();
}


static void setup() {
  int r;

  async_h = (uv_async_t*) malloc(sizeof(*async_h));
  assert(async_h != NULL);

  r = uv_async_init(uv_default_loop(), async_h, async_cb);
  assert(r == 0);
}


static void cleanup() {
  free(async_h);
}


int main() {
  int r;
  setup();
  run_test();
  r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  cleanup();
  printf("cntr: %i\n", cntr);
  return r;
}
