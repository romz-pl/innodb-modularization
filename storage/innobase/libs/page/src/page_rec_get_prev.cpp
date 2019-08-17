#include <innodb/page/page_rec_get_prev.h>

#include <innodb/page/page_rec_get_prev_const.h>

/** Gets the pointer to the previous record.
 @return pointer to previous record */
rec_t *page_rec_get_prev(
    rec_t *rec) /*!< in: pointer to record, must not be page
                infimum */
{
  return ((rec_t *)page_rec_get_prev_const(rec));
}
