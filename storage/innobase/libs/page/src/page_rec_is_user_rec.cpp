#include <innodb/page/page_rec_is_user_rec.h>

#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_is_user_rec_low.h>
#include <innodb/page/page_offset.h>
#include <innodb/assert/assert.h>

/** TRUE if the record is a user record on the page.
 @return true if a user record */
ibool page_rec_is_user_rec(const rec_t *rec) /*!< in: record */
{
  ut_ad(page_rec_check(rec));

  return (page_rec_is_user_rec_low(page_offset(rec)));
}
