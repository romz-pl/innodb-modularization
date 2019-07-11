#include <innodb/tablespace/fsp_header_get_page_size.h>

#include <innodb/tablespace/fsp_header_get_flags.h>

/** Reads the page size from the first page of a tablespace.
@param[in]	page	first page of a tablespace
@return page size */
page_size_t fsp_header_get_page_size(const page_t *page) {
  return (page_size_t(fsp_header_get_flags(page)));
}
