#ifndef __EASY_ALLOC_H__
#define __EASY_ALLOC_H__

typedef struct _EasyAllocNode {
  struct _EasyAllocNode *next;
  unsigned size;
} EasyAllocNode;

#define EASY_ALLOC_HEAD_SIZE (sizeof(void *) * 2)
#define EASY_ALLOC_NODE_SIZE EASY_ALLOC_HEAD_SIZE * 2

typedef struct _EasyAlloc {
  EasyAllocNode head;
} EasyAlloc;

static inline void EasyAllocInit(EasyAlloc *alloc) {
  alloc->head.next = 0;
  alloc->head.size = 0;
}

static inline unsigned EasyAllocRemaining(EasyAlloc *alloc) {
  return alloc->head.size;
}

static inline void _EasyAllocAddBlock(EasyAllocNode *head, EasyAllocNode *block) {
  EasyAllocNode *p = head->next;
  EasyAllocNode *pp = head;
  while (p) {
    if (p->size >= block->size) {
      break;
    }
    pp = p;
    p = p->next;
  }
  pp->next = block;
  block->next = p;
}

static inline void EasyAllocAddBlock(EasyAlloc *alloc, EasyAllocNode *block) {
  alloc->head.size += block->size;
  _EasyAllocAddBlock(&alloc->head, block);
}

static inline void EasyAllocAdd(EasyAlloc *alloc, void *ptr, unsigned size) {
  EasyAllocNode *p = (EasyAllocNode *)ptr;
  p->size = size;
  EasyAllocAddBlock(alloc, p);
}

static inline void EasyAllocFree(EasyAlloc *alloc, void *ptr) {
  EasyAllocNode *p = (EasyAllocNode *)((char *)ptr - EASY_ALLOC_HEAD_SIZE);
  EasyAllocAddBlock(alloc, p);
}

static inline void *EasyAllocAlloc(EasyAlloc *alloc, unsigned size) {
  EasyAllocNode *c;
  EasyAllocNode *p = alloc->head.next;
  EasyAllocNode *pp = &alloc->head;
  if (size > 0) {
    size += EASY_ALLOC_HEAD_SIZE;
    size = (size + EASY_ALLOC_NODE_SIZE - 1) & ~(unsigned)(EASY_ALLOC_NODE_SIZE - 1);

    while (p) {
      if (p->size < size) {
        pp = p;
        p = p->next;
      } else {
        pp->next = p->next;
        alloc->head.size -= p->size;

        if (p->size >= size + EASY_ALLOC_NODE_SIZE) {
          c = (EasyAllocNode *)((char *)p + size);
          c->size = p->size - size;
          p->size = size;
          EasyAllocAddBlock(alloc, c);
        }
        return (char *)p + EASY_ALLOC_HEAD_SIZE;
      }
    }
  }
  return 0;
}

static inline void EasyAllocMerge(EasyAlloc *alloc) {
  EasyAllocNode *p = alloc->head.next, *pp = &alloc->head;
  EasyAllocNode *p1, *pp1, *lastpp = &alloc->head;
  while (p) {
    p1 = alloc->head.next;
    pp1 = &alloc->head;
    while (p1) {
      if ((char *)p + p->size == (char *)p1) {
        p->size += p1->size;
        if (pp != p1) {
          pp1->next = p1->next;
          pp->next = p->next;
        } else {
          pp1->next = p->next;
          lastpp = pp = pp1;
        }
        _EasyAllocAddBlock(pp, p);
        p = 0;
        break;
      }
      pp1 = p1;
      p1 = p1->next;
    }
    if (p) {
      lastpp = pp = p;
      p = p->next;
    } else {
      p = pp->next;
    }
  }
}

static inline void *EasyAllocMalloc(EasyAlloc *alloc, unsigned size) {
  void *r = EasyAllocAlloc(alloc, size);
  if (r == NULL) {
    EasyAllocMerge(alloc);
    r = EasyAllocAlloc(alloc, size);
  }
  return r;
}

#endif // __EASY_ALLOC_H__
