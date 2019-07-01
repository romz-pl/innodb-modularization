#include <innodb/page/page_zip_des_t.h>

#include <innodb/assert/assert.h>
#include <innodb/math/ut_is_2pow.h>

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

/** Set the size of a compressed page in bytes. */
void page_zip_set_size(page_zip_des_t *page_zip, /*!< in/out: compressed page */
                       ulint size)               /*!< in: size in bytes */
{
  if (size) {
    int ssize;

    ut_ad(ut_is_2pow(size));

    for (ssize = 1; size > (ulint)(512 << ssize); ssize++) {
    }

    page_zip->ssize = ssize;
  } else {
    page_zip->ssize = 0;
  }

  ut_ad(page_zip_get_size(page_zip) == size);
}

/** Initialize a compressed page descriptor. */
void page_zip_des_init(page_zip_des_t *page_zip) /*!< in/out: compressed page
                                                 descriptor */
{
  memset(page_zip, 0, sizeof *page_zip);
}
