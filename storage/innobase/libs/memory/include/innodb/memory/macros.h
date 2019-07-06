#pragma once

#include <innodb/univ/univ.h>

#include <innodb/align/ut_calc_align.h>

/** Types of allocation for memory heaps: DYNAMIC means allocation from the
dynamic memory pool of the C compiler, BUFFER means allocation from the
buffer pool; the latter method is used for very big heaps */

#define MEM_HEAP_DYNAMIC 0 /* the most common type */
#define MEM_HEAP_BUFFER 1
#define MEM_HEAP_BTR_SEARCH            \
  2 /* this flag can optionally be     \
    ORed to MEM_HEAP_BUFFER, in which  \
    case heap->free_block is used in   \
    some cases for memory allocations, \
    and if it's NULL, the memory       \
    allocation functions can return    \
    NULL. */

/** Different type of heaps in terms of which data structure is using them */
#define MEM_HEAP_FOR_BTR_SEARCH (MEM_HEAP_BTR_SEARCH | MEM_HEAP_BUFFER)
#define MEM_HEAP_FOR_PAGE_HASH (MEM_HEAP_DYNAMIC)
#define MEM_HEAP_FOR_RECV_SYS (MEM_HEAP_BUFFER)
#define MEM_HEAP_FOR_LOCK_HEAP (MEM_HEAP_BUFFER)

/** The following start size is used for the first block in the memory heap if
the size is not specified, i.e., 0 is given as the parameter in the call of
create. The standard size is the maximum (payload) size of the blocks used for
allocations of small buffers. */

#define MEM_BLOCK_START_SIZE 64
#define MEM_BLOCK_STANDARD_SIZE \
  (UNIV_PAGE_SIZE >= 16384 ? 8000 : MEM_MAX_ALLOC_IN_BUF)

/** If a memory heap is allowed to grow into the buffer pool, the following
is the maximum size for a single allocated buffer
(from UNIV_PAGE_SIZE we subtract MEM_BLOCK_HEADER_SIZE and 2*MEM_NO_MANS_LAND
since it's something we always need to put. Since in MEM_SPACE_NEEDED we round
n to the next multiple of UNIV_MEM_ALINGMENT, we need to cut from the rest the
part that cannot be divided by UNIV_MEM_ALINGMENT): */
#define MEM_MAX_ALLOC_IN_BUF                                         \
  ((UNIV_PAGE_SIZE - MEM_BLOCK_HEADER_SIZE - 2 * MEM_NO_MANS_LAND) & \
   ~(UNIV_MEM_ALIGNMENT - 1))

/* Before and after any allocated object we will put MEM_NO_MANS_LAND bytes of
some data (different before and after) which is supposed not to be modified by
anyone. This way it would be much easier to determine whether anyone was
writing on not his memory, especially that Valgrind can assure there was no
reads or writes to this memory. */
#ifdef UNIV_DEBUG
const int MEM_NO_MANS_LAND = 16;
#else
const int MEM_NO_MANS_LAND = 0;
#endif

/* Byte that we would put before allocated object MEM_NO_MANS_LAND times.*/
const byte MEM_NO_MANS_LAND_BEFORE_BYTE = 0xCE;
/* Byte that we would put after allocated object MEM_NO_MANS_LAND times.*/
const byte MEM_NO_MANS_LAND_AFTER_BYTE = 0xDF;

/** Space needed when allocating for a user a field of length N.
The space is allocated only in multiples of UNIV_MEM_ALIGNMENT. In debug mode
contains two areas of no mans lands before and after the buffer requested. */
#define MEM_SPACE_NEEDED(N) \
  ut_calc_align(N + 2 * MEM_NO_MANS_LAND, UNIV_MEM_ALIGNMENT)

#define MEM_BLOCK_MAGIC_N 0x445566778899AABB
#define MEM_FREED_BLOCK_MAGIC_N 0xBBAA998877665544
