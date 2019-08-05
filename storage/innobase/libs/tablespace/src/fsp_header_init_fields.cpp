#include <innodb/tablespace/fsp_header_init_fields.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/tablespace/fsp_flags_is_valid.h>
#include <innodb/tablespace/consts.h>

/** Writes the space id and flags to a tablespace header.  The flags contain
 row type, physical/compressed page size, and logical/uncompressed page
 size of the tablespace. */
void fsp_header_init_fields(
    page_t *page,        /*!< in/out: first page in the space */
    space_id_t space_id, /*!< in: space id */
    uint32_t flags)      /*!< in: tablespace flags
                      (FSP_SPACE_FLAGS) */
{
  ut_a(fsp_flags_is_valid(flags));

  mach_write_to_4(FSP_HEADER_OFFSET + FSP_SPACE_ID + page, space_id);
  mach_write_to_4(FSP_HEADER_OFFSET + FSP_SPACE_FLAGS + page, flags);
}
