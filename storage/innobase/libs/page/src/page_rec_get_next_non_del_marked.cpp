#include <innodb/page/page_rec_get_next_non_del_marked.h>

#include <innodb/page/page_rec_is_comp.h>
#include <innodb/page/page_rec_get_next_const.h>
#include <innodb/page/page_rec_is_supremum.h>
#include <innodb/page/page_rec_get_next_const.h>
#include <innodb/record/rec_get_deleted_flag.h>


/** Gets the pointer to the next non delete-marked record on the page.
 If all subsequent records are delete-marked, then this function
 will return the supremum record.
 @return pointer to next non delete-marked record or pointer to supremum */
const rec_t *page_rec_get_next_non_del_marked(
    const rec_t *rec) /*!< in: pointer to record */
{
  const rec_t *r;
  ulint page_is_compact = page_rec_is_comp(rec);

  for (r = page_rec_get_next_const(rec);
       !page_rec_is_supremum(r) && rec_get_deleted_flag(r, page_is_compact);
       r = page_rec_get_next_const(r)) {
    /* noop */
  }

  return (r);
}
