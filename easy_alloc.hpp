#pragma once

#include <functional>

#include "easy_alloc.h"

class CEasyAlloc {
protected:
  EasyAlloc _alloc;
public:
  CEasyAlloc() {
    EasyAllocInit(&_alloc);
  }
  unsigned Remaining() {
    return EasyAllocRemaining(&_alloc);
  }
  void AddBlock(void *mem, unsigned size) {
    EasyAllocAdd(&_alloc, mem, size);
  }
  void *Alloc(unsigned size) {
    return EasyAllocAlloc(&_alloc, size);
  }
  void *Malloc(unsigned size) {
    return EasyAllocMalloc(&_alloc, size);
  }
  void Free(void *ptr) {
    EasyAllocFree(&_alloc, ptr);
  }
  void Merge() {
    EasyAllocMerge(&_alloc);
  }
  // DEBUG
  void ForEachBlock(const std::function<void(void *)> &fn) {
    for (EasyAllocNode *p = _alloc.head.next; p; ) {
      EasyAllocNode *node = p;
      p = p->next;

      fn(node);
    }
  }
};
