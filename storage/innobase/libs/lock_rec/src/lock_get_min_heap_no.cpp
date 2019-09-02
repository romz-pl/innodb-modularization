#include <innodb/lock_rec/lock_get_min_heap_no.h>

#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_heap_no_old.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/record/rec_get_next_offs.h>
#include <innodb/page/header.h>
#include <innodb/buf_block/buf_block_t.h>


/** Gets the heap_no of the smallest user record on a page.
 @return heap_no of smallest user record, or PAGE_HEAP_NO_SUPREMUM */
ulint lock_get_min_heap_no(const buf_block_t *block) /*!< in: buffer block */
{
  const page_t *page = block->frame;

  if (page_is_comp(page)) {
    return (rec_get_heap_no_new(
        page + rec_get_next_offs(page + PAGE_NEW_INFIMUM, TRUE)));
  } else {
    return (rec_get_heap_no_old(
        page + rec_get_next_offs(page + PAGE_OLD_INFIMUM, FALSE)));
  }
}
