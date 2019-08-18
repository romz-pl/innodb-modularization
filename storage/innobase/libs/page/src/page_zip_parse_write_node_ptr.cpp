#include <innodb/page/page_zip_parse_write_node_ptr.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/assert/assert.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/record/flag.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_dir_start.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/flag.h>


/** Parses a log record of writing the node pointer of a record.
 @return end of log record or NULL */
byte *page_zip_parse_write_node_ptr(
    byte *ptr,                /*!< in: redo log buffer */
    byte *end_ptr,            /*!< in: redo log buffer end */
    page_t *page,             /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip) /*!< in/out: compressed page */
{
  ulint offset;
  ulint z_offset;

  ut_ad(ptr != NULL);
  ut_ad(end_ptr != NULL);
  ut_ad(!page == !page_zip);

  if (UNIV_UNLIKELY(end_ptr < ptr + (2 + 2 + REC_NODE_PTR_SIZE))) {
    return (NULL);
  }

  offset = mach_read_from_2(ptr);
  z_offset = mach_read_from_2(ptr + 2);

  if (offset < PAGE_ZIP_START || offset >= UNIV_PAGE_SIZE ||
      z_offset >= UNIV_PAGE_SIZE) {
  corrupt:
    recv_sys->found_corrupt_log = TRUE;

    return (NULL);
  }

  if (page) {
    byte *storage_end;
    byte *field;
    byte *storage;
    ulint heap_no;

    if (!page_zip || page_is_leaf(page)) {
      goto corrupt;
    }

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */

    field = page + offset;
    storage = page_zip->data + z_offset;

    storage_end = page_zip_dir_start(page_zip);

    heap_no = 1 + (storage_end - storage) / REC_NODE_PTR_SIZE;

    if (UNIV_UNLIKELY((storage_end - storage) % REC_NODE_PTR_SIZE) ||
        UNIV_UNLIKELY(heap_no < PAGE_HEAP_NO_USER_LOW) ||
        UNIV_UNLIKELY(heap_no >= page_dir_get_n_heap(page))) {
      goto corrupt;
    }

    memcpy(field, ptr + 4, REC_NODE_PTR_SIZE);
    memcpy(storage, ptr + 4, REC_NODE_PTR_SIZE);

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */
  }

  return (ptr + (2 + 2 + REC_NODE_PTR_SIZE));
}
