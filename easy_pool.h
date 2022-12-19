#ifndef __EASY_POOL_H__
#define __EASY_POOL_H__

typedef struct _EasyPoolNode {
  struct _EasyPoolNode *next;
  unsigned size;
} EasyPoolNode;

#define EASY_POOL_NODE_SIZE (sizeof(void *) * 2)

typedef struct _EasyPool {
  EasyPoolNode head;
} EasyPool;

static inline void EasyPoolInit(EasyPool *pool) {
  pool->head.next = 0;
  pool->head.size = 0;
}

static inline unsigned EasyPoolSize(EasyPool *pool) {
  return pool->head.size;
}

static inline void _EasyPoolAddBlock(EasyPoolNode *head, EasyPoolNode *block) {
  EasyPoolNode *p = head->next;
  EasyPoolNode *pp = head;
  while (p) {
    if (p->size > block->size || (p->size == block->size && block - p < 0)) {
      break;
    }
    pp = p;
    p = p->next;
  }
  pp->next = block;
  block->next = p;
}

static inline void EasyPoolAddBlock(EasyPool *pool, EasyPoolNode *block) {
  pool->head.size += block->size;
  _EasyPoolAddBlock(&pool->head, block);
}

static inline void EasyPoolAdd(EasyPool *pool, void *ptr, unsigned size) {
  EasyPoolNode *p = (EasyPoolNode *)ptr;
  p->size = size;
  EasyPoolAddBlock(pool, p);
}

static inline void EasyPoolFree(EasyPool *pool, void *ptr) {
  EasyPoolNode *p = (EasyPoolNode *)((char *)ptr - EASY_POOL_NODE_SIZE);
  EasyPoolAddBlock(pool, p);
}

static inline void *EasyPoolAlloc(EasyPool *pool, unsigned size) {
  EasyPoolNode *c;
  EasyPoolNode *p = pool->head.next;
  EasyPoolNode *pp = &pool->head;
  if (size > 0) {
    size += EASY_POOL_NODE_SIZE;
    size = (size + 15) & ~(unsigned)15;

    while (p) {
      if (p->size < size) {
        pp = p;
        p = p->next;
      } else {
        pp->next = p->next;
        pool->head.size -= p->size;

        if (p->size >= size + 64) {
          c = (EasyPoolNode *)((char *)p + size);
          c->size = p->size - size;
          p->size = size;
          EasyPoolAddBlock(pool, c);
        }
        return (char *)p + EASY_POOL_NODE_SIZE;
      }
    }
  }
  return 0;
}

static inline void EasyPoolMerge(EasyPool *pool) {
  EasyPoolNode *p = pool->head.next, *pp = &pool->head;
  EasyPoolNode *p1, *pp1, *lastpp = &pool->head;
  while (p) {
    p1 = pool->head.next;
    pp1 = &pool->head;
    while (p1) {
      if ((char *)p + p->size == (char *)p1) {
        p->size += p1->size;
        if (p1 != p->next) {
          pp->next = p->next;
          pp1->next = p1->next;
        } else {
          pp->next = p1->next;
        }
        _EasyPoolAddBlock(pp, p);
        p = NULL;
        break;
      }
      pp1 = p1;
      p1 = p1->next;
    }
    if (p) {
      lastpp = pp = p;
      p = p->next;
    } else {
      pp = lastpp;
      p = pp->next;
    }
  }
}

#endif // __EASY_POOL_H__