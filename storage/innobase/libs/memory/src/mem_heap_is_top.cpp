#include <innodb/memory/mem_heap_is_top.h>

#include <innodb/memory/mem_heap_get_heap_top.h>
#include <innodb/memory/mem_block_info_t.h>
#include <innodb/memory/macros.h>
#include <innodb/memory/mem_block_validate.h>


/** Checks if a given chunk of memory is the topmost element stored in the
heap. If this is the case, then calling mem_heap_free_top() would free
that element from the heap.
@param[in]	heap	memory heap
@param[in]	buf	presumed topmost element
@param[in]	buf_sz	size of buf in bytes
@return true if topmost */
bool mem_heap_is_top(mem_heap_t *heap, const void *buf, ulint buf_sz) {
  const byte *first_free_byte;
  const byte *presumed_start_of_buf;

  ut_d(mem_block_validate(heap));

  first_free_byte = mem_heap_get_heap_top(heap);

  presumed_start_of_buf =
      first_free_byte - MEM_SPACE_NEEDED(buf_sz) + MEM_NO_MANS_LAND;

  return (presumed_start_of_buf == buf);
}
