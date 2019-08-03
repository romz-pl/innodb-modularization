#include <innodb/page/page_is_root.h>

#include <innodb/page/header.h>
#include <innodb/page/fil_page_index_page_check.h>
#include <innodb/page/page_no_t.h>

/** Determine whether a page is an index root page.
@param[in]	page	page frame
@return true if the page is a root page of an index */
bool page_is_root(const page_t *page) {
#if FIL_PAGE_PREV % 8
#error FIL_PAGE_PREV must be 64-bit aligned
#endif
#if FIL_PAGE_NEXT != FIL_PAGE_PREV + 4
#error FIL_PAGE_NEXT must be adjacent to FIL_PAGE_PREV
#endif

  static_assert(FIL_NULL == 0xffffffff, "FIL_NULL != 0xffffffff");

  /* Check that this is an index page and both the PREV and NEXT
  pointers are FIL_NULL, because the root page does not have any
  siblings. */
  return (fil_page_index_page_check(page) &&
          *reinterpret_cast<const ib_uint64_t *>(page + FIL_PAGE_PREV) ==
              IB_UINT64_MAX);
}
