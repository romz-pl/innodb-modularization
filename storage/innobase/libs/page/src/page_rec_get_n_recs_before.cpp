#include <innodb/page/page_rec_get_n_recs_before.h>

#include <innodb/page/header.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_next_ptr_const.h>
#include <innodb/record/rec_get_n_owned_old.h>


/** Returns the number of records before the given record in chain.
 The number includes infimum and supremum records.
 @return number of records */
ulint page_rec_get_n_recs_before(
    const rec_t *rec) /*!< in: the physical record */
{
  const page_dir_slot_t *slot;
  const rec_t *slot_rec;
  const page_t *page;
  ulint i;
  lint n = 0;

  ut_ad(page_rec_check(rec));

  page = page_align(rec);
  if (page_is_comp(page)) {
    while (rec_get_n_owned_new(rec) == 0) {
      rec = rec_get_next_ptr_const(rec, TRUE);
      n--;
    }

    for (i = 0;; i++) {
      slot = page_dir_get_nth_slot(page, i);
      slot_rec = page_dir_slot_get_rec(slot);

      n += rec_get_n_owned_new(slot_rec);

      if (rec == slot_rec) {
        break;
      }
    }
  } else {
    while (rec_get_n_owned_old(rec) == 0) {
      rec = rec_get_next_ptr_const(rec, FALSE);
      n--;
    }

    for (i = 0;; i++) {
      slot = page_dir_get_nth_slot(page, i);
      slot_rec = page_dir_slot_get_rec(slot);

      n += rec_get_n_owned_old(slot_rec);

      if (rec == slot_rec) {
        break;
      }
    }
  }

  n--;

  ut_ad(n >= 0);
  ut_ad((ulong)n < UNIV_PAGE_SIZE / (REC_N_NEW_EXTRA_BYTES + 1));

  return ((ulint)n);
}

