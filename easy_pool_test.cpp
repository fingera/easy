#include <stdio.h>

#include "easy_pool.h"

void EasyPoolDump(EasyPool *pool) {
  printf("easy pool dump %u\n", EasyPoolSize(pool));
  for (EasyPoolNode *p = pool->head.next; p; p = p->next) {
    printf("\tblock(%p, %u)\n", p, p->size);
  }
}

int main(void) {
  char data[4096];
  EasyPool pool;
  void *p, *p1, *p2, *p3;

  EasyPoolInit(&pool);
  EasyPoolAdd(&pool, data, sizeof(data));

  EasyPoolDump(&pool);
  p = EasyPoolAlloc(&pool, 33);
  EasyPoolDump(&pool);
  EasyPoolFree(&pool, p);
  EasyPoolDump(&pool);
  p = EasyPoolAlloc(&pool, 33);
  p1 = EasyPoolAlloc(&pool, 33);
  p2 = EasyPoolAlloc(&pool, 99);
  p3 = EasyPoolAlloc(&pool, 22);
  EasyPoolDump(&pool);
  EasyPoolFree(&pool, p);
  EasyPoolFree(&pool, p2);
  EasyPoolDump(&pool);
  EasyPoolMerge(&pool);
  EasyPoolDump(&pool);
  EasyPoolFree(&pool, p3);
  EasyPoolFree(&pool, p1);
  EasyPoolDump(&pool);
  EasyPoolMerge(&pool);
  EasyPoolDump(&pool);

  return 0;
}