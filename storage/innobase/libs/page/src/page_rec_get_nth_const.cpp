#include <innodb/page/page_rec_get_nth_const.h>

#include <innodb/assert/assert.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_get_n_owned.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_get_infimum_rec.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_rec_get_next_low.h>


/** Returns the nth record of the record list.
 This is the inverse function of page_rec_get_n_recs_before().
 @return nth record */
const rec_t *page_rec_get_nth_const(const page_t *page, /*!< in: page */
                                    ulint nth)          /*!< in: nth record */
{
  const page_dir_slot_t *slot;
  ulint i;
  ulint n_owned;
  const rec_t *rec;

  if (nth == 0) {
    return (page_get_infimum_rec(page));
  }

  ut_ad(nth < UNIV_PAGE_SIZE / (REC_N_NEW_EXTRA_BYTES + 1));

  for (i = 0;; i++) {
    slot = page_dir_get_nth_slot(page, i);
    n_owned = page_dir_slot_get_n_owned(slot);

    if (n_owned > nth) {
      break;
    } else {
      nth -= n_owned;
    }
  }

  ut_ad(i > 0);
  slot = page_dir_get_nth_slot(page, i - 1);
  rec = page_dir_slot_get_rec(slot);

  if (page_is_comp(page)) {
    do {
      rec = page_rec_get_next_low(rec, TRUE);
      ut_ad(rec);
    } while (nth--);
  } else {
    do {
      rec = page_rec_get_next_low(rec, FALSE);
      ut_ad(rec);
    } while (nth--);
  }

  return (rec);
}
