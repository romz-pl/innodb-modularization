#include <innodb/page/page_rec_is_last.h>

#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_rec_get_next_const.h>
#include <innodb/page/page_get_supremum_rec.h>
#include <innodb/assert/assert.h>

/** true if the record is the last user record on a page.
 @return true if the last user record */
bool page_rec_is_last(const rec_t *rec,   /*!< in: record */
                      const page_t *page) /*!< in: page */
{
  ut_ad(page_get_n_recs(page) > 0);

  return (page_rec_get_next_const(rec) == page_get_supremum_rec(page));
}
