#include <innodb/page/page_zip_set_size.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/assert/assert.h>
#include <innodb/math/ut_is_2pow.h>


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
