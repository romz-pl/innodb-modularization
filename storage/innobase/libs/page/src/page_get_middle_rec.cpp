#include <innodb/page/page_get_middle_rec.h>

#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_rec_get_nth.h>
#include <innodb/page/flag.h>

#ifndef UNIV_HOTBACKUP
/** Returns the middle record of the records on the page. If there is an
 even number of records in the list, returns the first record of the
 upper half-list.
 @return middle record */
rec_t *page_get_middle_rec(page_t *page) /*!< in: page */
{
  ulint middle = (page_get_n_recs(page) + PAGE_HEAP_NO_USER_LOW) / 2;

  return (page_rec_get_nth(page, middle));
}
#endif /* !UNIV_HOTBACKUP */
