#include <innodb/page/page_rec_is_supremum.h>

#include <innodb/page/page_rec_is_supremum_low.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/assert/assert.h>

/** TRUE if the record is the supremum record on a page.
 @return true if the supremum record */
ibool page_rec_is_supremum(const rec_t *rec) /*!< in: record */
{
  ut_ad(page_rec_check(rec));

  return (page_rec_is_supremum_low(page_offset(rec)));
}
