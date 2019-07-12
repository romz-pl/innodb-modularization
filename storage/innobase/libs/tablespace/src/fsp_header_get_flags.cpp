#include <innodb/tablespace/fsp_header_get_flags.h>

#include <innodb/tablespace/fsp_header_get_field.h>
#include <innodb/tablespace/consts.h>

/** Read the flags from the tablespace header page.
@param[in]	page	first page of a tablespace
@return the contents of FSP_SPACE_FLAGS */
ulint fsp_header_get_flags(const page_t *page) {
  return (fsp_header_get_field(page, FSP_SPACE_FLAGS));
}
