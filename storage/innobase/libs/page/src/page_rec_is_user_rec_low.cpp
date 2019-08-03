#include <innodb/page/page_rec_is_user_rec_low.h>

#include <innodb/assert/assert.h>
#include <innodb/page/header.h>

/** TRUE if the record is a user record on the page.
 @return true if a user record */
ibool page_rec_is_user_rec_low(ulint offset) /*!< in: record offset on page */
{
  ut_ad(offset >= PAGE_NEW_INFIMUM);

  static_assert(PAGE_OLD_INFIMUM >= PAGE_NEW_INFIMUM,
                "PAGE_OLD_INFIMUM < PAGE_NEW_INFIMUM");

  static_assert(PAGE_OLD_SUPREMUM >= PAGE_NEW_SUPREMUM,
                "PAGE_OLD_SUPREMUM < PAGE_NEW_SUPREMUM");

  static_assert(PAGE_NEW_INFIMUM <= PAGE_OLD_SUPREMUM,
                "PAGE_NEW_INFIMUM > PAGE_OLD_SUPREMUM");

  static_assert(PAGE_OLD_INFIMUM <= PAGE_NEW_SUPREMUM,
                "PAGE_OLD_INFIMUM > PAGE_NEW_SUPREMUM");

  static_assert(PAGE_NEW_SUPREMUM <= PAGE_OLD_SUPREMUM_END,
                "PAGE_NEW_SUPREMUM > PAGE_OLD_SUPREMUM_END");

  static_assert(PAGE_OLD_SUPREMUM <= PAGE_NEW_SUPREMUM_END,
                "PAGE_OLD_SUPREMUM > PAGE_NEW_SUPREMUM_END");

  ut_ad(offset <= UNIV_PAGE_SIZE - PAGE_EMPTY_DIR_START);

  return (offset != PAGE_NEW_SUPREMUM && offset != PAGE_NEW_INFIMUM &&
          offset != PAGE_OLD_INFIMUM && offset != PAGE_OLD_SUPREMUM);
}
