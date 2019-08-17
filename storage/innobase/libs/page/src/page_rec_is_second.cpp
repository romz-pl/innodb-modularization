#include <innodb/page/page_rec_is_second.h>

#include <innodb/assert/assert.h>
#include <innodb/page/page_get_infimum_rec.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_rec_get_next_const.h>
#include <innodb/page/page_rec_get_next_const.h>

/** true if the record is the second user record on a page.
 @return true if the second user record */
bool page_rec_is_second(const rec_t *rec,   /*!< in: record */
                        const page_t *page) /*!< in: page */
{
  ut_ad(page_get_n_recs(page) > 1);

  return (page_rec_get_next_const(
              page_rec_get_next_const(page_get_infimum_rec(page))) == rec);
}
