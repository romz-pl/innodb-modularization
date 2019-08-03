#include <innodb/page/page_get_n_recs.h>

#include <innodb/page/page_header_get_field.h>
#include <innodb/page/header.h>

/** Gets the number of user records on page (infimum and supremum records
 are not user records).
 @return number of user records */
ulint page_get_n_recs(const page_t *page) /*!< in: index page */
{
  return (page_header_get_field(page, PAGE_N_RECS));
}
