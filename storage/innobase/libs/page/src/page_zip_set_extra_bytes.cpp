#include <innodb/page/page_zip_set_extra_bytes.h>

#include <innodb/compiler_hints/compiler_hints.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_zip_dir_get.h>
#include <innodb/page/page_zip_dir_get.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/record/rec_set_next_offs_new.h>
#include <innodb/univ/rec_t.h>


/** Initialize the REC_N_NEW_EXTRA_BYTES of each record.
 @return true on success, false on failure */
ibool page_zip_set_extra_bytes(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    page_t *page,                   /*!< in/out: uncompressed page */
    ulint info_bits)                /*!< in: REC_INFO_MIN_REC_FLAG or 0 */
{
  ulint n;
  ulint i;
  ulint n_owned = 1;
  ulint offs;
  rec_t *rec;

  n = page_get_n_recs(page);
  rec = page + PAGE_NEW_INFIMUM;

  for (i = 0; i < n; i++) {
    offs = page_zip_dir_get(page_zip, i);

    if (offs & PAGE_ZIP_DIR_SLOT_DEL) {
      info_bits |= REC_INFO_DELETED_FLAG;
    }
    if (UNIV_UNLIKELY(offs & PAGE_ZIP_DIR_SLOT_OWNED)) {
      info_bits |= n_owned;
      n_owned = 1;
    } else {
      n_owned++;
    }
    offs &= PAGE_ZIP_DIR_SLOT_MASK;
    if (UNIV_UNLIKELY(offs < PAGE_ZIP_START + REC_N_NEW_EXTRA_BYTES)) {
      page_zip_fail(
          ("page_zip_set_extra_bytes 1:"
           " %u %u %lx\n",
           (unsigned)i, (unsigned)n, (ulong)offs));
      return (FALSE);
    }

    rec_set_next_offs_new(rec, offs);
    rec = page + offs;
    rec[-REC_N_NEW_EXTRA_BYTES] = (byte)info_bits;
    info_bits = 0;
  }

  /* Set the next pointer of the last user record. */
  rec_set_next_offs_new(rec, PAGE_NEW_SUPREMUM);

  /* Set n_owned of the supremum record. */
  page[PAGE_NEW_SUPREMUM - REC_N_NEW_EXTRA_BYTES] = (byte)n_owned;

  /* The dense directory excludes the infimum and supremum records. */
  n = page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW;

  if (i >= n) {
    if (UNIV_LIKELY(i == n)) {
      return (TRUE);
    }

    page_zip_fail(
        ("page_zip_set_extra_bytes 2: %u != %u\n", (unsigned)i, (unsigned)n));
    return (FALSE);
  }

  offs = page_zip_dir_get(page_zip, i);

  /* Set the extra bytes of deleted records on the free list. */
  for (;;) {
    if (UNIV_UNLIKELY(!offs) || UNIV_UNLIKELY(offs & ~PAGE_ZIP_DIR_SLOT_MASK)) {
      page_zip_fail(("page_zip_set_extra_bytes 3: %lx\n", (ulong)offs));
      return (FALSE);
    }

    rec = page + offs;
    rec[-REC_N_NEW_EXTRA_BYTES] = 0; /* info_bits and n_owned */

    if (++i == n) {
      break;
    }

    offs = page_zip_dir_get(page_zip, i);
    rec_set_next_offs_new(rec, offs);
  }

  /* Terminate the free list. */
  rec[-REC_N_NEW_EXTRA_BYTES] = 0; /* info_bits and n_owned */
  rec_set_next_offs_new(rec, 0);

  return (TRUE);
}
