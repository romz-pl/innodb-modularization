#include <innodb/tablespace/fsp_header_get_field.h>

#include <innodb/machine/data.h>
#include <innodb/tablespace/header.h>

/** Read a tablespace header field.
@param[in]	page	first page of a tablespace
@param[in]	field	the header field
@return the contents of the header field */
uint32_t fsp_header_get_field(const page_t *page, ulint field) {
  return (mach_read_from_4(FSP_HEADER_OFFSET + field + page));
}
