#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_new_pfx_t.h>

struct buf_block_t;

/** A chunk of buffers. The buffer pool is allocated in chunks. */
struct buf_chunk_t {
  ulint size;           /*!< size of frames[] and blocks[] */
  unsigned char *mem;   /*!< pointer to the memory area which
                        was allocated for the frames */
  ut_new_pfx_t mem_pfx; /*!< Auxiliary structure, describing
                        "mem". It is filled by the allocator's
                        alloc method and later passed to the
                        deallocate method. */
  buf_block_t *blocks;  /*!< array of buffer control blocks */

  /** Get the size of 'mem' in bytes. */
  size_t mem_size() const { return (mem_pfx.m_size); }

  /** Advices the OS that this chunk should not be dumped to a core file.
  Emits a warning to the log if could not succeed.
  @return true iff succeeded, false if no OS support or failed */
  bool madvise_dump();

  /** Advices the OS that this chunk should be dumped to a core file.
  Emits a warning to the log if could not succeed.
  @return true iff succeeded, false if no OS support or failed */
  bool madvise_dont_dump();
};
