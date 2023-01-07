#ifndef __EASY_RINGBUF_H__
#define __EASY_RINGBUF_H__

typedef struct _EasyRingBufShared {
  EASY_U32 read;
  EASY_U32 write;
  EASY_U32 pad0;
  EASY_U32 pad1;
} EasyRingBufShared;

#define EASY_RINGBUF_MEM_OFFSET 16

typedef struct _EasyRingBuf {
  EasyRingBufShared *head;
  char *mem;
  unsigned size;
} EasyRingBuf;

static inline void EasyRingBufInit(EasyRingBuf *ring, void *shared, unsigned size) {
  ring->head = (EasyRingBufShared *)shared;
  ring->mem = (char *)shared + EASY_RINGBUF_MEM_OFFSET;
  ring->size = size - EASY_RINGBUF_MEM_OFFSET;
}

static inline int EasyRingBufCanRead(EasyRingBuf *ring) {
  EASY_U32 write = EASY_ATOMIC_LOAD_ACQUIRE(ring->head->write);
  EASY_U32 read = ring->head->read;
  return read == write ? 0 : 1;
}
static inline int EasyRingBufCanWrite(EasyRingBuf *ring) {
  EASY_U32 read = EASY_ATOMIC_LOAD_ACQUIRE(ring->head->read);
  EASY_U32 write = ring->head->write;
  if (write == read - 1) {
    return 0;
  }
  if (read == 0 && write == ring->size - 1) {
    return 0;
  }
  return 1;
}

static inline unsigned EasyRingBufRead(EasyRingBuf *ring, void *buf, unsigned size) {
  unsigned readed, remaining, pos;
  char *ptr = (char *)buf;

  EASY_U32 write = EASY_ATOMIC_LOAD_ACQUIRE(ring->head->write);
  EASY_U32 read = ring->head->read;

  if (read == write) {
    readed = 0;

  } else if (read < write) {
    readed = write - read;
    if (readed > size) readed = size;
    EASY_MEMCPY(ptr, ring->mem + read, readed);
    EASY_ATOMIC_STORE_RELEASE(ring->head->read, read + readed);

  } else {
    readed = ring->size - read;

    if (readed >= size) {
      readed = size;
      EASY_MEMCPY(ptr, ring->mem + read, readed);
      pos = read + readed;
      EASY_ATOMIC_STORE_RELEASE(ring->head->read, pos == ring->size ? 0 : pos);

    } else {
      EASY_MEMCPY(ptr, ring->mem + read, readed);

      remaining = size - readed;
      if (remaining > write) remaining = write;
      if (remaining > 0) {
        EASY_MEMCPY(ptr + readed, ring->mem, remaining);
        readed += remaining;
      }
      EASY_ATOMIC_STORE_RELEASE(ring->head->read, remaining);
    }
  }
  return readed;
}

static inline unsigned EasyRingBufWrite(EasyRingBuf *ring, void *buf, unsigned size) {
  unsigned wrote, remaining, pos;
  char *ptr = (char *)buf;

  EASY_U32 read = EASY_ATOMIC_LOAD_ACQUIRE(ring->head->read);
  EASY_U32 write = ring->head->write;

  if (write < read) {
    wrote = (read - 1) - write;
    if (wrote > size) wrote = size;
    if (wrote > 0) {
      EASY_MEMCPY(ring->mem + write, ptr, wrote);
      EASY_ATOMIC_STORE_RELEASE(ring->head->write, ring->head->write + wrote);
    }

  } else {
    wrote = ring->size - write;
    if (read == 0) wrote--;

    if (wrote >= size) {
      wrote = size;
      EASY_MEMCPY(ring->mem + write, ptr, wrote);
      pos = write + wrote;
      EASY_ATOMIC_STORE_RELEASE(ring->head->write, pos == ring->size ? 0 : pos);

    } else {
      if (wrote > 0) EASY_MEMCPY(ring->mem + write, ptr, wrote);
      if (read > 0) {
        remaining = size - wrote;
        if (remaining > read - 1) remaining = read - 1;
        if (remaining > 0) {
          EASY_MEMCPY(ring->mem, ptr + wrote, remaining);
          wrote += remaining;
        }
      } else {
        remaining = wrote + write;
      }
      EASY_ATOMIC_STORE_RELEASE(ring->head->write, remaining);
    }
  }
  return wrote;
}

#endif // __EASY_RINGBUF_H__