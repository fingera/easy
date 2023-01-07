#pragma once

#include <cstring>
#include <cstdint>

#define EASY_U32 uint32_t
#define EASY_MEMCPY memcpy
// MSVC in arm InterlockedXXX
#define EASY_ATOMIC_LOAD_ACQUIRE(v) __atomic_load_n(&v, __ATOMIC_ACQUIRE)
#define EASY_ATOMIC_STORE_RELEASE(v, value) __atomic_store_n(&v, value, __ATOMIC_RELEASE)

#include "easy_ringbuf.h"

class EasyRingBufNull {
public:
  EasyRingBufNull(size_t maxSize) {}
  inline void Wait(unsigned readed, unsigned wrote, bool forRead) {}
  inline void Wakeup(unsigned readed, unsigned wrote) {}
};

template<typename WAIT>
class CEasyRingBuf {
protected:
  EasyRingBuf _ring;
  WAIT _wait;
public:
  CEasyRingBuf(void *sharedBuffer, unsigned len) : _wait(len - EASY_RINGBUF_MEM_OFFSET - 1) {
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
    unsigned readed = 0, tmp = 0;
    while (readed < len) {
      unsigned bytes = Read((char *)buf + readed, len - readed);
      if (bytes == 0) {
        _wait.Wait(tmp, 0, true);
        tmp = 0;
      }
      readed += bytes;
      tmp += bytes;
    }
    _wait.Wakeup(tmp, 0);
  }
  void WriteLoop(const void *buf, unsigned len) {
    unsigned wrote = 0, tmp = 0;
    while (wrote < len) {
      unsigned bytes = Write((char *)buf + wrote, len - wrote);
      if (bytes == 0) {
        _wait.Wait(0, tmp, false);
        tmp = 0;
      }
      wrote += bytes;
      tmp += bytes;
    }
    _wait.Wakeup(0, tmp);
  }
  template<typename SIMPLE>
  void WriteLoop(const SIMPLE &value) {
    WriteLoop(&value, sizeof(value));
  }
  template<typename SIMPLE>
  void ReadLoop(SIMPLE &value) {
    ReadLoop(&value, sizeof(value));
  }
};
