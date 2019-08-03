#include <innodb/page/page_rec_is_supremum_low.h>

#include <innodb/page/header.h>
#include <innodb/assert/assert.h>
#include <innodb/page/flag.h>

/** TRUE if the record is the supremum record on a page.
 @return true if the supremum record */
ibool page_rec_is_supremum_low(ulint offset) /*!< in: record offset on page */
{
  ut_ad(offset >= PAGE_NEW_INFIMUM);
  ut_ad(offset <= UNIV_PAGE_SIZE - PAGE_EMPTY_DIR_START);

  return (offset == PAGE_NEW_SUPREMUM || offset == PAGE_OLD_SUPREMUM);
}
