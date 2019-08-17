#include <innodb/page/page_rec_get_prev_const.h>

#include <innodb/assert/assert.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_find_owner_slot.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_dir_slot_t.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_get_next_low.h>
#include <innodb/page/page_rec_get_next_low.h>
#include <innodb/page/page_rec_is_infimum.h>


/** Gets the pointer to the previous record.
 @return pointer to previous record */
const rec_t *page_rec_get_prev_const(
    const rec_t *rec) /*!< in: pointer to record, must not be page
                      infimum */
{
  const page_dir_slot_t *slot;
  ulint slot_no;
  const rec_t *rec2;
  const rec_t *prev_rec = NULL;
  const page_t *page;

  ut_ad(page_rec_check(rec));

  page = page_align(rec);

  ut_ad(!page_rec_is_infimum(rec));

  slot_no = page_dir_find_owner_slot(rec);

  ut_a(slot_no != 0);

  slot = page_dir_get_nth_slot(page, slot_no - 1);

  rec2 = page_dir_slot_get_rec(slot);

  if (page_is_comp(page)) {
    while (rec != rec2) {
      prev_rec = rec2;
      rec2 = page_rec_get_next_low(rec2, TRUE);
    }
  } else {
    while (rec != rec2) {
      prev_rec = rec2;
      rec2 = page_rec_get_next_low(rec2, FALSE);
    }
  }

  ut_a(prev_rec);

  return (prev_rec);
}
