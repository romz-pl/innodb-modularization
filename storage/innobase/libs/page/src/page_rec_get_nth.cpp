#include <innodb/page/page_rec_get_nth.h>

#include <innodb/page/page_rec_get_nth_const.h>

/** Returns the nth record of the record list.
 This is the inverse function of page_rec_get_n_recs_before().
 @return nth record */
rec_t *page_rec_get_nth(page_t *page, /*!< in: page */
                        ulint nth)    /*!< in: nth record */
{
  return ((rec_t *)page_rec_get_nth_const(page, nth));
}
