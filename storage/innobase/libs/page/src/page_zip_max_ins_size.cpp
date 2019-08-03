#include <innodb/page/page_zip_max_ins_size.h>

#include <innodb/page/flag.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_trailer_len.h>
#include <innodb/record/flag.h>

/** Determine how big record can be inserted without recompressing the page.
 @return a positive number indicating the maximum size of a record
 whose insertion is guaranteed to succeed, or zero or negative */
lint page_zip_max_ins_size(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    ibool is_clust)                 /*!< in: TRUE if clustered index */
{
  ulint trailer_len;

  trailer_len = page_zip_get_trailer_len(page_zip, is_clust);

  /* When a record is created, a pointer may be added to
  the dense directory.
  Likewise, space for the columns that will not be
  compressed will be allocated from the page trailer.
  Also the BLOB pointers will be allocated from there, but
  we may as well count them in the length of the record. */

  trailer_len += PAGE_ZIP_DIR_SLOT_SIZE;

  return ((lint)page_zip_get_size(page_zip) - trailer_len - page_zip->m_end -
          (REC_N_NEW_EXTRA_BYTES - 2));
}
