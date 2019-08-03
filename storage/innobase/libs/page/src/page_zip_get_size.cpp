#include <innodb/page/page_zip_get_size.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/assert/assert.h>

/** Determine the size of a compressed page in bytes.
 @return size in bytes */
ulint page_zip_get_size(
    const page_zip_des_t *page_zip) /*!< in: compressed page */
{
  ulint size;

  if (!page_zip->ssize) {
    return (0);
  }

  size = (UNIV_ZIP_SIZE_MIN >> 1) << page_zip->ssize;

  ut_ad(size >= UNIV_ZIP_SIZE_MIN);
  ut_ad(size <= UNIV_PAGE_SIZE);

  return (size);
}
