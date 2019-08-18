#include <innodb/page/page_zip_dir_insert.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_rec_is_infimum.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_find.h>
#include <innodb/page/page_zip_dir_find_low.h>
#include <innodb/page/page_zip_dir_user_size.h>
#include <innodb/page/page_zip_get_size.h>


/** Insert a record to the dense page directory. */
void page_zip_dir_insert(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *prev_rec,     /*!< in: record after which to insert */
    const byte *free_rec,     /*!< in: record from which rec was
                             allocated, or NULL */
    byte *rec)                /*!< in: record to insert */
{
  ulint n_dense;
  byte *slot_rec;
  byte *slot_free;

  ut_ad(prev_rec != rec);
  ut_ad(page_rec_get_next((rec_t *)prev_rec) == rec);
  ut_ad(page_zip_simple_validate(page_zip));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  if (page_rec_is_infimum(prev_rec)) {
    /* Use the first slot. */
    slot_rec = page_zip->data + page_zip_get_size(page_zip);
  } else {
    byte *end = page_zip->data + page_zip_get_size(page_zip);
    byte *start = end - page_zip_dir_user_size(page_zip);

    if (UNIV_LIKELY(!free_rec)) {
      /* PAGE_N_RECS was already incremented
      in page_cur_insert_rec_zip(), but the
      dense directory slot at that position
      contains garbage.  Skip it. */
      start += PAGE_ZIP_DIR_SLOT_SIZE;
    }

    slot_rec = page_zip_dir_find_low(start, end, page_offset(prev_rec));
    ut_a(slot_rec);
  }

  /* Read the old n_dense (n_heap may have been incremented). */
  n_dense = page_dir_get_n_heap(page_zip->data) - (PAGE_HEAP_NO_USER_LOW + 1);

  if (UNIV_LIKELY_NULL(free_rec)) {
    /* The record was allocated from the free list.
    Shift the dense directory only up to that slot.
    Note that in this case, n_dense is actually
    off by one, because page_cur_insert_rec_zip()
    did not increment n_heap. */
    ut_ad(rec_get_heap_no_new(rec) < n_dense + 1 + PAGE_HEAP_NO_USER_LOW);
    ut_ad(rec >= free_rec);
    slot_free = page_zip_dir_find(page_zip, page_offset(free_rec));
    ut_ad(slot_free);
    slot_free += PAGE_ZIP_DIR_SLOT_SIZE;
  } else {
    /* The record was allocated from the heap.
    Shift the entire dense directory. */
    ut_ad(rec_get_heap_no_new(rec) == n_dense + PAGE_HEAP_NO_USER_LOW);

    /* Shift to the end of the dense page directory. */
    slot_free = page_zip->data + page_zip_get_size(page_zip) -
                PAGE_ZIP_DIR_SLOT_SIZE * n_dense;
  }

  /* Shift the dense directory to allocate place for rec. */
  memmove(slot_free - PAGE_ZIP_DIR_SLOT_SIZE, slot_free, slot_rec - slot_free);

  /* Write the entry for the inserted record.
  The "owned" and "deleted" flags must be zero. */
  mach_write_to_2(slot_rec - PAGE_ZIP_DIR_SLOT_SIZE, page_offset(rec));
}
