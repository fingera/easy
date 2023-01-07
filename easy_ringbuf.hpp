#pragma once

#include <cstring>
#include <cstdint>
#include <thread> // std::this_thread::yield()

#define EASY_U32 uint32_t
#define EASY_MEMCPY memcpy
// MSVC in arm InterlockedXXX
#define EASY_ATOMIC_LOAD_ACQUIRE(v) __atomic_load_n(&v, __ATOMIC_ACQUIRE)
#define EASY_ATOMIC_STORE_RELEASE(v, value) __atomic_store_n(&v, value, __ATOMIC_RELEASE)

#include "easy_ringbuf.h"

class CEasyRingBuf {
protected:
  EasyRingBuf _ring;
public:
  CEasyRingBuf(void *sharedBuffer, unsigned len) {
    memset(sharedBuffer, 0, EASY_RINGBUF_MEM_OFFSET);
    EasyRingBufInit(&_ring, sharedBuffer, len);
  }
  unsigned Read(void *buf, unsigned len) {
    return EasyRingBufRead(&_ring, buf, len);
  }
  unsigned Write(void *buf, unsigned len) {
    return EasyRingBufWrite(&_ring, buf, len);
  }

  void ReadLoop(void *buf, unsigned len) {
    unsigned readed = 0;
    while (readed < len) {
      unsigned bytes = Read((char *)buf + readed, len - readed);
      if (bytes == 0) std::this_thread::yield();
      readed += bytes;
    }
  }
  void WriteLoop(const void *buf, unsigned len) {
    unsigned wrote = 0;
    while (wrote < len) {
      unsigned bytes = Write((char *)buf + wrote, len - wrote);
      if (bytes == 0) std::this_thread::yield();
      wrote += bytes;
    }
  }
  template<typename T>
  void WriteLoop(const T &value) {
    WriteLoop(&value, sizeof(value));
  }
  template<typename T>
  void ReadLoop(T &value) {
    ReadLoop(&value, sizeof(value));
  }
};
