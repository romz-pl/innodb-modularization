#include <innodb/page/page_zip_parse_write_header.h>

#include <innodb/assert/assert.h>
#include <innodb/page/header.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/page_zip_des_t.h>

/** Parses a log record of writing to the header of a page.
 @return end of log record or NULL */
byte *page_zip_parse_write_header(
    byte *ptr,                /*!< in: redo log buffer */
    byte *end_ptr,            /*!< in: redo log buffer end */
    page_t *page,             /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip) /*!< in/out: compressed page */
{
  ulint offset;
  ulint len;

  ut_ad(ptr != NULL);
  ut_ad(end_ptr != NULL);
  ut_ad(!page == !page_zip);

  if (UNIV_UNLIKELY(end_ptr < ptr + (1 + 1))) {
    return (NULL);
  }

  offset = (ulint)*ptr++;
  len = (ulint)*ptr++;

  if (len == 0 || offset + len >= PAGE_DATA) {
  corrupt:
    recv_sys->found_corrupt_log = TRUE;

    return (NULL);
  }

  if (end_ptr < ptr + len) {
    return (NULL);
  }

  if (page) {
    if (!page_zip) {
      goto corrupt;
    }
#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */

    memcpy(page + offset, ptr, len);
    memcpy(page_zip->data + offset, ptr, len);

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */
  }

  return (ptr + len);
}

