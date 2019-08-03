#include <innodb/tablespace/xdes_calc_descriptor_page.h>

#include <innodb/tablespace/consts.h>
#include <innodb/math/ut_2pow_round.h>
#include <innodb/assert/assert.h>
#include <innodb/disk/page_size_t.h>

/** Calculates the page where the descriptor of a page resides.
@param[in]	page_size	page size
@param[in]	offset		page offset
@return descriptor page offset */
page_no_t xdes_calc_descriptor_page(const page_size_t &page_size,
                                    page_no_t offset) {
  static_assert(UNIV_PAGE_SIZE_MAX > XDES_ARR_OFFSET + (UNIV_PAGE_SIZE_MAX /
                                                        FSP_EXTENT_SIZE_MAX) *
                                                           XDES_SIZE_MAX,
                "Extent descriptor won't fit on a page");

  static_assert(UNIV_ZIP_SIZE_MIN > XDES_ARR_OFFSET + (UNIV_ZIP_SIZE_MIN /
                                                       FSP_EXTENT_SIZE_MIN) *
                                                          XDES_SIZE_MIN,
                "Extent descriptor won't fit on a page");

  ut_ad(UNIV_PAGE_SIZE >
        XDES_ARR_OFFSET + (UNIV_PAGE_SIZE / FSP_EXTENT_SIZE) * XDES_SIZE);
  ut_ad(UNIV_ZIP_SIZE_MIN >
        XDES_ARR_OFFSET + (UNIV_ZIP_SIZE_MIN / FSP_EXTENT_SIZE) * XDES_SIZE);

#ifdef UNIV_DEBUG
  if (page_size.is_compressed()) {
    ut_a(page_size.physical() >
         XDES_ARR_OFFSET +
             (page_size.physical() / FSP_EXTENT_SIZE) * XDES_SIZE);
  }
#endif /* UNIV_DEBUG */

  return (ut_2pow_round(offset, page_size.physical()));
}
