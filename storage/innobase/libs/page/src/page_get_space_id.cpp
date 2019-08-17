#include <innodb/page/page_get_space_id.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/disk/flags.h>

/** Gets the tablespace identifier.
 @return space id */
space_id_t page_get_space_id(const page_t *page) /*!< in: page */
{
  ut_ad(page == page_align((page_t *)page));
  return (mach_read_from_4(page + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID));
}
