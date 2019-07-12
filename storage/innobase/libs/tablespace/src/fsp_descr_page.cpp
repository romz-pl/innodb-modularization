#include <innodb/tablespace/fsp_descr_page.h>

#include <innodb/tablespace/page_id_t.h>
#include <innodb/page/page_size_t.h>
#include <innodb/tablespace/consts.h>

/** Checks if a page address is an extent descriptor page address.
@param[in]	page_id		page id
@param[in]	page_size	page size
@return true if a descriptor page */
ibool fsp_descr_page(const page_id_t &page_id, const page_size_t &page_size) {
  return ((page_id.page_no() & (page_size.physical() - 1)) == FSP_XDES_OFFSET);
}
