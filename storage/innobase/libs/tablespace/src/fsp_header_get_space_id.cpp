#include <innodb/tablespace/fsp_header_get_space_id.h>

#include <innodb/logger/error.h>
#include <innodb/machine/data.h>
#include <innodb/tablespace/consts.h>

/** Reads the space id from the first page of a tablespace.
 @return space id, ULINT UNDEFINED if error */
space_id_t fsp_header_get_space_id(
    const page_t *page) /*!< in: first page of a tablespace */
{
  space_id_t fsp_id;
  space_id_t id;

  fsp_id = mach_read_from_4(FSP_HEADER_OFFSET + page + FSP_SPACE_ID);

  id = mach_read_from_4(page + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);

  DBUG_EXECUTE_IF("fsp_header_get_space_id_failure", id = SPACE_UNKNOWN;);

  if (id != fsp_id) {
    ib::error(ER_IB_MSG_414) << "Space ID in fsp header is " << fsp_id
                             << ", but in the page header it is " << id << ".";
    return (SPACE_UNKNOWN);
  }

  return (id);
}
