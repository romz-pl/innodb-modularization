#include <innodb/buf_pool/buf_ptr_get_fsp_addr.h>

#include <innodb/align/ut_align_down.h>
#include <innodb/align/ut_align_offset.h>
#include <innodb/buf_page/flags.h>
#include <innodb/machine/data.h>
#include <innodb/disk/page_t.h>
#include <innodb/disk/flags.h>
#include <innodb/tablespace/fil_addr_t.h>

/** Gets the space id, page offset, and byte offset within page of a
 pointer pointing to a buffer frame containing a file page. */
void buf_ptr_get_fsp_addr(
    const void *ptr,   /*!< in: pointer to a buffer frame */
    space_id_t *space, /*!< out: space id */
    fil_addr_t *addr)  /*!< out: page offset and byte offset */
{
  const page_t *page = (const page_t *)ut_align_down(ptr, UNIV_PAGE_SIZE);

  *space = mach_read_from_4(page + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);
  addr->page = mach_read_from_4(page + FIL_PAGE_OFFSET);
  addr->boffset = static_cast<uint32_t>(ut_align_offset(ptr, UNIV_PAGE_SIZE));
}
