#include <innodb/page/page_zip_available.h>

#include <innodb/assert/assert.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_trailer_len.h>
#include <innodb/record/flag.h>

/** Determine if enough space is available in the modification log.
 @return true if enough space is available */
ibool page_zip_available(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    bool is_clust,                  /*!< in: TRUE if clustered index */
    ulint length,                   /*!< in: combined size of the record */
    ulint create)                   /*!< in: nonzero=add the record to
                                    the heap */
{
  ulint trailer_len;

  ut_ad(length > REC_N_NEW_EXTRA_BYTES);

  trailer_len = page_zip_get_trailer_len(page_zip, is_clust);

  /* Subtract the fixed extra bytes and add the maximum
  space needed for identifying the record (encoded heap_no). */
  length -= REC_N_NEW_EXTRA_BYTES - 2;

  if (create > 0) {
    /* When a record is created, a pointer may be added to
    the dense directory.
    Likewise, space for the columns that will not be
    compressed will be allocated from the page trailer.
    Also the BLOB pointers will be allocated from there, but
    we may as well count them in the length of the record. */

    trailer_len += PAGE_ZIP_DIR_SLOT_SIZE;
  }

  return (length + trailer_len + page_zip->m_end < page_zip_get_size(page_zip));
}

