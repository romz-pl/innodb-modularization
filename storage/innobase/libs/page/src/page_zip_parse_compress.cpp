#include <innodb/page/page_zip_parse_compress.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_decompress.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/disk/page_type_t.h>

/** Parses a log record of compressing an index page.
 @return end of log record or NULL */
byte *page_zip_parse_compress(
    byte *ptr,                /*!< in: buffer */
    byte *end_ptr,            /*!< in: buffer end */
    page_t *page,             /*!< out: uncompressed page */
    page_zip_des_t *page_zip) /*!< out: compressed page */
{
  ulint size;
  ulint trailer_size;

  ut_ad(ptr != NULL);
  ut_ad(end_ptr != NULL);
  ut_ad(!page == !page_zip);

  if (UNIV_UNLIKELY(ptr + (2 + 2) > end_ptr)) {
    return (NULL);
  }

  size = mach_read_from_2(ptr);
  ptr += 2;
  trailer_size = mach_read_from_2(ptr);
  ptr += 2;

  if (UNIV_UNLIKELY(ptr + 8 + size + trailer_size > end_ptr)) {
    return (NULL);
  }

  if (page) {
    if (!page_zip || page_zip_get_size(page_zip) < size) {
    corrupt:
      recv_sys->found_corrupt_log = TRUE;

      return (NULL);
    }

    memcpy(page_zip->data + FIL_PAGE_PREV, ptr, 4);
    memcpy(page_zip->data + FIL_PAGE_NEXT, ptr + 4, 4);
    memcpy(page_zip->data + FIL_PAGE_TYPE, ptr + 8, size);
    memset(page_zip->data + FIL_PAGE_TYPE + size, 0,
           page_zip_get_size(page_zip) - trailer_size - (FIL_PAGE_TYPE + size));
    memcpy(page_zip->data + page_zip_get_size(page_zip) - trailer_size,
           ptr + 8 + size, trailer_size);

    if (UNIV_UNLIKELY(!page_zip_decompress(page_zip, page, TRUE))) {
      goto corrupt;
    }
  }

  return (ptr + 8 + size + trailer_size);
}
