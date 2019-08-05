#pragma once

#include <innodb/univ/univ.h>

//#include <innodb/memory/mem_block_t.h>
#include <innodb/lst/lst.h>

/** A block of a memory heap consists of the info structure
followed by an area of memory */
typedef struct mem_block_info_t mem_block_t;

/** The info structure stored at the beginning of a heap block */
struct mem_block_info_t {
  uint64_t magic_n; /* magic number for debugging */
#ifdef UNIV_DEBUG
  char file_name[16]; /* file name where the mem heap was created */
  ulint line;         /*!< line number where the mem heap was created */
#endif                /* UNIV_DEBUG */
  UT_LIST_BASE_NODE_T(mem_block_t)
  base; /* In the first block in the
the list this is the base node of the list of blocks;
in subsequent blocks this is undefined */
  UT_LIST_NODE_T(mem_block_t)
  list;             /* This contains pointers to next
  and prev in the list. The first block allocated
  to the heap is also the first block in this list,
  though it also contains the base node of the list. */
  ulint len;        /*!< physical length of this block in bytes */
  ulint total_size; /*!< physical length in bytes of all blocks
                in the heap. This is defined only in the base
                node and is set to ULINT_UNDEFINED in others. */
  ulint type;       /*!< type of heap: MEM_HEAP_DYNAMIC, or
                    MEM_HEAP_BUF possibly ORed to MEM_HEAP_BTR_SEARCH */
  ulint free;       /*!< offset in bytes of the first free position for
                    user data in the block */
  ulint start;      /*!< the value of the struct field 'free' at the
                    creation of the block */
  void *free_block;
  /* if the MEM_HEAP_BTR_SEARCH bit is set in type,
  and this is the heap root, this can contain an
  allocated buffer frame, which can be appended as a
  free block to the heap, if we need more space;
  otherwise, this is NULL */
  void *buf_block;
  /* if this block has been allocated from the buffer
  pool, this contains the buf_block_t handle;
  otherwise, this is NULL */
};

