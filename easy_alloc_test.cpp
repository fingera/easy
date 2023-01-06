#include <stdio.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "easy_alloc.hpp"

CEasyAlloc gAlloc;
std::mutex gAllocMutex;

void *my_malloc(size_t size) {
  std::lock_guard<std::mutex> guard(gAllocMutex);
  return gAlloc.Malloc(size);
}
void my_free(void *ptr) {
  std::lock_guard<std::mutex> guard(gAllocMutex);
  gAlloc.Free(ptr);
}

void test() {
  std::vector<void *> bufs;
  for (int i = 0; i < 10000; i++) {
    auto size = rand();
    void *buf = my_malloc(size);
    if (buf != NULL) {
      bufs.push_back(buf);
      memset(buf, -1, size);
    } else if (!bufs.empty()) {
      size_t idx = bufs.size() > 1 ? (rand() % bufs.size()) : 0;
      my_free(bufs[idx]);
      bufs.erase(bufs.begin() + idx);
    }
  }
  for (auto i = bufs.begin(); i != bufs.end(); ++i) {
    my_free(*i);
  }
}

int main(void) {
  const int BufferCount = 32;
  const int BufferPagesLimit = 256;
  const int ThreadLoopCount = 10000;
  const int ThreadCount  = 32;

  std::unordered_map<void *, size_t> bufs;
  
  printf("head: %d node: %d\n", (int)EASY_ALLOC_HEAD_SIZE, (int)EASY_ALLOC_NODE_SIZE);

  for (int i = 0; i < BufferCount; i++) {
    int size = (rand() % BufferPagesLimit) * 4096;
    void *sysmem = malloc(size);
    if (sysmem == NULL) break;
    bufs.insert(std::make_pair(sysmem, size));
    gAlloc.AddBlock(sysmem, (unsigned)size);
  }
  printf("initialized\n");

  std::vector<std::thread> threads;
  for (int i = 0; i < ThreadCount; i++) {
    threads.emplace_back(test);
  }
  for (auto &t : threads) {
    t.join();
  }
  printf("tested\n");

  gAlloc.Merge();
  gAlloc.ForEachBlock([&bufs] (void *block) {
    auto finded = bufs.find(block);
    while (finded != bufs.end()) {
      printf("free: %p %lx\n", finded->first, finded->second);
      free(finded->first);
      char *ptr = (char *)finded->first + finded->second;
      bufs.erase(finded);
      finded = bufs.find(ptr);
    }
  });

  if (bufs.empty()) {
    printf("success\n");
  } else {
    for (auto &buf : bufs) {
      printf("%p %lx\n", buf.first, buf.second);
    }
    printf("bufs error\n");
  }

  return 0;
}