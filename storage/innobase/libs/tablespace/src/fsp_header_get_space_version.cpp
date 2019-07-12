#include <innodb/tablespace/fsp_header_get_space_version.h>

#include <innodb/machine/data.h>
#include <innodb/tablespace/consts.h>

/** Reads the server space version from the first page of a tablespace.
@param[in]      page            first page of a tablespace
@return space server version */
uint32 fsp_header_get_space_version(const page_t *page) {
  uint32 version;

  version = mach_read_from_4(page + FIL_PAGE_SPACE_VERSION);

  return (version);
}
