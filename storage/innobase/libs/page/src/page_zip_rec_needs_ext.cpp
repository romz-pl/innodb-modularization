#include <innodb/page/page_zip_rec_needs_ext.h>

#include <innodb/page/page_zip_empty_size.h>
#include <innodb/page/page_get_free_space_of_empty.h>
#include <innodb/assert/assert.h>
#include <innodb/record/flag.h>


#ifndef UNIV_HOTBACKUP
/** Determine if a record is so big that it needs to be stored externally.
@param[in]	rec_size	length of the record in bytes
@param[in]	comp		nonzero=compact format
@param[in]	n_fields	number of fields in the record; ignored if
tablespace is not compressed
@param[in]	page_size	page size
@return false if the entire record can be stored locally on the page */
ibool page_zip_rec_needs_ext(ulint rec_size, ulint comp, ulint n_fields,
                             const page_size_t &page_size) {
  ut_ad(rec_size > (comp ? REC_N_NEW_EXTRA_BYTES : REC_N_OLD_EXTRA_BYTES));
  ut_ad(comp || !page_size.is_compressed());

  if (UNIV_PAGE_SIZE_MAX > REC_MAX_DATA_SIZE && rec_size >= REC_MAX_DATA_SIZE) {
    return (TRUE);
  }

  if (page_size.is_compressed()) {
    ut_ad(comp);
    /* On a compressed page, there is a two-byte entry in
    the dense page directory for every record.  But there
    is no record header.  There should be enough room for
    one record on an empty leaf page.  Subtract 1 byte for
    the encoded heap number.  Check also the available space
    on the uncompressed page. */
    return (rec_size - (REC_N_NEW_EXTRA_BYTES - 2 - 1) >=
                page_zip_empty_size(n_fields, page_size.physical()) ||
            rec_size >= page_get_free_space_of_empty(TRUE) / 2);
  }

  return (rec_size >= page_get_free_space_of_empty(comp) / 2);
}
#endif /* !UNIV_HOTBACKUP */
