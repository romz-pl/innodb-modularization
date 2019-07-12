#include <innodb/tablespace/fsp_init.h>

#include <innodb/tablespace/consts.h>
#include <innodb/assert/assert.h>

/** Initializes the fsp system. */
void fsp_init() {
  /* FSP_EXTENT_SIZE must be a multiple of page & zip size */
  ut_a(UNIV_PAGE_SIZE > 0);
  ut_a(0 == (UNIV_PAGE_SIZE % FSP_EXTENT_SIZE));

  static_assert(!(UNIV_PAGE_SIZE_MAX % FSP_EXTENT_SIZE_MAX),
                "UNIV_PAGE_SIZE_MAX % FSP_EXTENT_SIZE_MAX != 0");

  static_assert(!(UNIV_ZIP_SIZE_MIN % FSP_EXTENT_SIZE_MIN),
                "UNIV_ZIP_SIZE_MIN % FSP_EXTENT_SIZE_MIN != 0");

  /* Does nothing at the moment */
}
