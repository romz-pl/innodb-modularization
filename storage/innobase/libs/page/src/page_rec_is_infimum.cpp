#include <innodb/page/page_rec_is_infimum.h>

#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_is_infimum_low.h>
#include <innodb/page/page_offset.h>
#include <innodb/assert/assert.h>

/** TRUE if the record is the infimum record on a page.
 @return true if the infimum record */
ibool page_rec_is_infimum(const rec_t *rec) /*!< in: record */
{
  ut_ad(page_rec_check(rec));

  return (page_rec_is_infimum_low(page_offset(rec)));
}
