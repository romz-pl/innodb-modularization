#include <innodb/memory/mem_heap_free_top.h>

#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_block_set_free.h>
#include <innodb/memory/validate_no_mans_land.h>
#include <innodb/memory/mem_block_get_free.h>
#include <innodb/memory/mem_heap_block_free.h>
#include <innodb/memory/mem_block_get_start.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/assert/assert.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/memory/macros.h>

/** Frees the topmost element in a memory heap. The size of the element must be
 given. */
void mem_heap_free_top(mem_heap_t *heap, /*!< in: memory heap */
                       ulint n)          /*!< in: size of the topmost element */
{
  mem_block_t *block;

  ut_d(mem_block_validate(heap));

  block = UT_LIST_GET_LAST(heap->base);

  /* Subtract the free field of block */
  mem_block_set_free(block, mem_block_get_free(block) - MEM_SPACE_NEEDED(n));
  UNIV_MEM_ASSERT_W(
      (byte *)block + mem_block_get_free(block) + MEM_NO_MANS_LAND, n);
#ifdef UNIV_DEBUG
  validate_no_mans_land((byte *)block + mem_block_get_free(block),
                        MEM_NO_MANS_LAND_BEFORE_BYTE);
  validate_no_mans_land((byte *)block + mem_block_get_free(block) +
                            MEM_SPACE_NEEDED(n) - MEM_NO_MANS_LAND,
                        MEM_NO_MANS_LAND_AFTER_BYTE);
#endif

  /* If free == start, we may free the block if it is not the first
  one */

  if ((heap != block) &&
      (mem_block_get_free(block) == mem_block_get_start(block))) {
    mem_heap_block_free(heap, block);
  } else {
    UNIV_MEM_FREE((byte *)block + mem_block_get_free(block),
                  MEM_SPACE_NEEDED(n));
  }
}

