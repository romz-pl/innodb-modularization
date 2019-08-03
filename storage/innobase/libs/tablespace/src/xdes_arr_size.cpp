#include <innodb/tablespace/xdes_arr_size.h>

#include <innodb/disk/page_size_t.h>
#include <innodb/tablespace/consts.h>

/** Calculates the descriptor array size.
@param[in]	page_size	page size
@return size of descriptor array */
ulint xdes_arr_size(const page_size_t &page_size) {
  return (page_size.physical() / FSP_EXTENT_SIZE);
}
