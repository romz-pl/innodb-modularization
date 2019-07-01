#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_zip_t.h>


/** Compressed page descriptor */
struct page_zip_des_t {
  page_zip_t *data; /*!< compressed page data */

#ifdef UNIV_DEBUG
  unsigned m_start : 16;   /*!< start offset of modification log */
  bool m_external;         /*!< Allocated externally, not from the
                           buffer pool */
#endif                     /* UNIV_DEBUG */
  unsigned m_end : 16;     /*!< end offset of modification log */
  unsigned m_nonempty : 1; /*!< TRUE if the modification log
                           is not empty */
  unsigned n_blobs : 12;   /*!< number of externally stored
                           columns on the page; the maximum
                           is 744 on a 16 KiB page */
  unsigned ssize : PAGE_ZIP_SSIZE_BITS;
  /*!< 0 or compressed page shift size;
  the size in bytes is
  (UNIV_ZIP_SIZE_MIN >> 1) << ssize. */
};

/** Determine the size of a compressed page in bytes.
 @return size in bytes */
ulint page_zip_get_size(
    const page_zip_des_t *page_zip); /*!< in: compressed page */

/** Set the size of a compressed page in bytes.
@param[in,out]	page_zip	compressed page
@param[in]	size		size in bytes */
void page_zip_set_size(page_zip_des_t *page_zip, ulint size);

/** Initialize a compressed page descriptor. */
void page_zip_des_init(page_zip_des_t *page_zip); /*!< in/out: compressed page
                                                  descriptor */

