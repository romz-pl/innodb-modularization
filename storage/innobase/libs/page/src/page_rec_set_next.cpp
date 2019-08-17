#include <innodb/page/page_rec_set_next.h>

#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_is_supremum.h>
#include <innodb/page/page_rec_is_infimum.h>
#include <innodb/page/page_rec_is_comp.h>
#include <innodb/record/rec_set_next_offs_new.h>
#include <innodb/record/rec_set_next_offs_old.h>
#include <innodb/page/page_align.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_offset.h>


/** Sets the pointer to the next record on the page. */
void page_rec_set_next(rec_t *rec,        /*!< in: pointer to record,
                                          must not be page supremum */
                       const rec_t *next) /*!< in: pointer to next record,
                                          must not be page infimum */
{
  ulint offs;

  ut_ad(page_rec_check(rec));
  ut_ad(!page_rec_is_supremum(rec));
  ut_ad(rec != next);

  ut_ad(!next || !page_rec_is_infimum(next));
  ut_ad(!next || page_align(rec) == page_align(next));

  offs = next != NULL ? page_offset(next) : 0;

  if (page_rec_is_comp(rec)) {
    rec_set_next_offs_new(rec, offs);
  } else {
    rec_set_next_offs_old(rec, offs);
  }
}
