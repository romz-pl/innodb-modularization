#include <innodb/page/page_rec_get_next_const.h>

#include <innodb/page/page_rec_get_next_low.h>
#include <innodb/page/page_rec_is_comp.h>


/** Gets the pointer to the next record on the page.
 @return pointer to next record */
const rec_t *page_rec_get_next_const(
    const rec_t *rec) /*!< in: pointer to record */
{
  return (page_rec_get_next_low(rec, page_rec_is_comp(rec)));
}
