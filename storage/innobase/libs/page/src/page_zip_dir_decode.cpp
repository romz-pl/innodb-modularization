#include <innodb/page/page_zip_dir_decode.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_dir_get_n_slots.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_t.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_get.h>
#include <innodb/page/page_zip_fail.h>

#include <algorithm>

/** Populate the sparse page directory from the dense directory.
 @return true on success, false on failure */
MY_ATTRIBUTE((warn_unused_result)) ibool page_zip_dir_decode(
    const page_zip_des_t *page_zip, /*!< in: dense page directory on
                                   compressed page */
    page_t *page,                   /*!< in: compact page with valid header;
                                    out: trailer and sparse page directory
                                    filled in */
    rec_t **recs,                   /*!< out: dense page directory sorted by
                                    ascending address (and heap_no) */
    ulint n_dense)                  /*!< in: number of user records, and
                                    size of recs[] */
{
  ulint i;
  ulint n_recs;
  byte *slot;

  n_recs = page_get_n_recs(page);

  if (UNIV_UNLIKELY(n_recs > n_dense)) {
    page_zip_fail(
        ("page_zip_dir_decode 1: %lu > %lu\n", (ulong)n_recs, (ulong)n_dense));
    return (FALSE);
  }

  /* Traverse the list of stored records in the sorting order,
  starting from the first user record. */

  slot = page + (UNIV_PAGE_SIZE - PAGE_DIR - PAGE_DIR_SLOT_SIZE);
  UNIV_PREFETCH_RW(slot);

  /* Zero out the page trailer. */
  memset(slot + PAGE_DIR_SLOT_SIZE, 0, PAGE_DIR);

  mach_write_to_2(slot, PAGE_NEW_INFIMUM);
  slot -= PAGE_DIR_SLOT_SIZE;
  UNIV_PREFETCH_RW(slot);

  /* Initialize the sparse directory and copy the dense directory. */
  for (i = 0; i < n_recs; i++) {
    ulint offs = page_zip_dir_get(page_zip, i);

    if (offs & PAGE_ZIP_DIR_SLOT_OWNED) {
      mach_write_to_2(slot, offs & PAGE_ZIP_DIR_SLOT_MASK);
      slot -= PAGE_DIR_SLOT_SIZE;
      UNIV_PREFETCH_RW(slot);
    }

    if (UNIV_UNLIKELY((offs & PAGE_ZIP_DIR_SLOT_MASK) <
                      PAGE_ZIP_START + REC_N_NEW_EXTRA_BYTES)) {
      page_zip_fail(("page_zip_dir_decode 2: %u %u %lx\n", (unsigned)i,
                     (unsigned)n_recs, (ulong)offs));
      return (FALSE);
    }

    recs[i] = page + (offs & PAGE_ZIP_DIR_SLOT_MASK);
  }

  mach_write_to_2(slot, PAGE_NEW_SUPREMUM);
  {
    const page_dir_slot_t *last_slot =
        page_dir_get_nth_slot(page, page_dir_get_n_slots(page) - 1);

    if (UNIV_UNLIKELY(slot != last_slot)) {
      page_zip_fail(("page_zip_dir_decode 3: %p != %p\n", (const void *)slot,
                     (const void *)last_slot));
      return (FALSE);
    }
  }

  /* Copy the rest of the dense directory. */
  for (; i < n_dense; i++) {
    ulint offs = page_zip_dir_get(page_zip, i);

    if (UNIV_UNLIKELY(offs & ~PAGE_ZIP_DIR_SLOT_MASK)) {
      page_zip_fail(("page_zip_dir_decode 4: %u %u %lx\n", (unsigned)i,
                     (unsigned)n_dense, (ulong)offs));
      return (FALSE);
    }

    recs[i] = page + offs;
  }

  std::sort(recs, recs + n_dense);
  return (TRUE);
}
