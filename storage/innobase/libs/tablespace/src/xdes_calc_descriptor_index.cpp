#include <innodb/tablespace/xdes_calc_descriptor_index.h>

#include <innodb/math/ut_2pow_remainder.h>
#include <innodb/tablespace/consts.h>
#include <innodb/disk/page_size_t.h>

/** Calculates the descriptor index within a descriptor page.
@param[in]	page_size	page size
@param[in]	offset		page offset
@return descriptor index */
ulint xdes_calc_descriptor_index(const page_size_t &page_size, ulint offset) {
  return (ut_2pow_remainder(offset, page_size.physical()) / FSP_EXTENT_SIZE);
}
