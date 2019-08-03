#include <innodb/page/page_zip_dir_encode.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_next_offs.h>
#include <innodb/record/rec_get_status.h>


/** Populate the dense page directory from the sparse directory. */
void page_zip_dir_encode(
    const page_t *page, /*!< in: compact page */
    byte *buf,          /*!< in: pointer to dense page directory[-1];
                        out: dense directory on compressed page */
    const rec_t **recs) /*!< in: pointer to an array of 0, or NULL;
                        out: dense page directory sorted by ascending
                        address (and heap_no) */
{
  const byte *rec;
  ulint status;
  ulint min_mark;
  ulint heap_no;
  ulint i;
  ulint n_heap;
  ulint offs;

  min_mark = 0;

  if (page_is_leaf(page)) {
    status = REC_STATUS_ORDINARY;
  } else {
    status = REC_STATUS_NODE_PTR;
    if (UNIV_UNLIKELY(mach_read_from_4(page + FIL_PAGE_PREV) == FIL_NULL)) {
      min_mark = REC_INFO_MIN_REC_FLAG;
    }
  }

  n_heap = page_dir_get_n_heap(page);

  /* Traverse the list of stored records in the collation order,
  starting from the first user record. */

  rec = page + PAGE_NEW_INFIMUM;

  i = 0;

  for (;;) {
    ulint info_bits;
    offs = rec_get_next_offs(rec, TRUE);
    if (UNIV_UNLIKELY(offs == PAGE_NEW_SUPREMUM)) {
      break;
    }
    rec = page + offs;
    heap_no = rec_get_heap_no_new(rec);
    ut_a(heap_no >= PAGE_HEAP_NO_USER_LOW);
    ut_a(heap_no < n_heap);
    ut_a(offs < UNIV_PAGE_SIZE - PAGE_DIR);
    ut_a(offs >= PAGE_ZIP_START);
#if PAGE_ZIP_DIR_SLOT_MASK & (PAGE_ZIP_DIR_SLOT_MASK + 1)
#error PAGE_ZIP_DIR_SLOT_MASK is not 1 less than a power of 2
#endif
#if PAGE_ZIP_DIR_SLOT_MASK < UNIV_ZIP_SIZE_MAX - 1
#error PAGE_ZIP_DIR_SLOT_MASK < UNIV_ZIP_SIZE_MAX - 1
#endif
    if (UNIV_UNLIKELY(rec_get_n_owned_new(rec))) {
      offs |= PAGE_ZIP_DIR_SLOT_OWNED;
    }

    info_bits = rec_get_info_bits(rec, TRUE);
    if (info_bits & REC_INFO_DELETED_FLAG) {
      info_bits &= ~REC_INFO_DELETED_FLAG;
      offs |= PAGE_ZIP_DIR_SLOT_DEL;
    }
    ut_a(info_bits == min_mark);
    /* Only the smallest user record can have
    REC_INFO_MIN_REC_FLAG set. */
    min_mark = 0;

    mach_write_to_2(buf - PAGE_ZIP_DIR_SLOT_SIZE * ++i, offs);

    if (UNIV_LIKELY_NULL(recs)) {
      /* Ensure that each heap_no occurs at most once. */
      ut_a(!recs[heap_no - PAGE_HEAP_NO_USER_LOW]);
      /* exclude infimum and supremum */
      recs[heap_no - PAGE_HEAP_NO_USER_LOW] = rec;
    }

    ut_a(rec_get_status(rec) == status);
  }

  offs = page_header_get_field(page, PAGE_FREE);

  /* Traverse the free list (of deleted records). */
  while (offs) {
    ut_ad(!(offs & ~PAGE_ZIP_DIR_SLOT_MASK));
    rec = page + offs;

    heap_no = rec_get_heap_no_new(rec);
    ut_a(heap_no >= PAGE_HEAP_NO_USER_LOW);
    ut_a(heap_no < n_heap);

    ut_a(!rec[-REC_N_NEW_EXTRA_BYTES]); /* info_bits and n_owned */
    ut_a(rec_get_status(rec) == status);

    mach_write_to_2(buf - PAGE_ZIP_DIR_SLOT_SIZE * ++i, offs);

    if (UNIV_LIKELY_NULL(recs)) {
      /* Ensure that each heap_no occurs at most once. */
      ut_a(!recs[heap_no - PAGE_HEAP_NO_USER_LOW]);
      /* exclude infimum and supremum */
      recs[heap_no - PAGE_HEAP_NO_USER_LOW] = rec;
    }

    offs = rec_get_next_offs(rec, TRUE);
  }

  /* Ensure that each heap no occurs at least once. */
  ut_a(i + PAGE_HEAP_NO_USER_LOW == n_heap);
}
