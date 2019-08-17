#include <innodb/page/page_dir_balance_slot.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/page/header.h>
#include <innodb/page/page_dir_delete_slot.h>
#include <innodb/page/page_dir_get_n_slots.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_get_n_owned.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_dir_slot_set_n_owned.h>
#include <innodb/page/page_dir_slot_set_rec.h>
#include <innodb/page/page_dir_slot_t.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/record/rec_get_next_ptr.h>
#include <innodb/record/rec_get_next_ptr.h>
#include <innodb/record/rec_get_next_ptr.h>
#include <innodb/record/rec_get_next_ptr.h>
#include <innodb/record/rec_set_n_owned_new.h>
#include <innodb/record/rec_set_n_owned_old.h>
#include <innodb/univ/rec_t.h>


/** Tries to balance the given directory slot with too few records with the
 upper neighbor, so that there are at least the minimum number of records owned
 by the slot; this may result in the merging of two slots. */
void page_dir_balance_slot(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    ulint slot_no)            /*!< in: the directory slot */
{
  page_dir_slot_t *slot;
  page_dir_slot_t *up_slot;
  ulint n_owned;
  ulint up_n_owned;
  rec_t *old_rec;
  rec_t *new_rec;

  ut_ad(page);
  ut_ad(!page_zip || page_is_comp(page));
  ut_ad(slot_no > 0);

  slot = page_dir_get_nth_slot(page, slot_no);

  /* The last directory slot cannot be balanced with the upper
  neighbor, as there is none. */

  if (UNIV_UNLIKELY(slot_no == page_dir_get_n_slots(page) - 1)) {
    return;
  }

  up_slot = page_dir_get_nth_slot(page, slot_no + 1);

  n_owned = page_dir_slot_get_n_owned(slot);
  up_n_owned = page_dir_slot_get_n_owned(up_slot);

  ut_ad(n_owned == PAGE_DIR_SLOT_MIN_N_OWNED - 1);

  /* If the upper slot has the minimum value of n_owned, we will merge
  the two slots, therefore we assert: */
  ut_ad(2 * PAGE_DIR_SLOT_MIN_N_OWNED - 1 <= PAGE_DIR_SLOT_MAX_N_OWNED);

  if (up_n_owned > PAGE_DIR_SLOT_MIN_N_OWNED) {
    /* In this case we can just transfer one record owned
    by the upper slot to the property of the lower slot */
    old_rec = (rec_t *)page_dir_slot_get_rec(slot);

    if (page_is_comp(page)) {
      new_rec = rec_get_next_ptr(old_rec, TRUE);

      rec_set_n_owned_new(old_rec, page_zip, 0);
      rec_set_n_owned_new(new_rec, page_zip, n_owned + 1);
    } else {
      new_rec = rec_get_next_ptr(old_rec, FALSE);

      rec_set_n_owned_old(old_rec, 0);
      rec_set_n_owned_old(new_rec, n_owned + 1);
    }

    page_dir_slot_set_rec(slot, new_rec);

    page_dir_slot_set_n_owned(up_slot, page_zip, up_n_owned - 1);
  } else {
    /* In this case we may merge the two slots */
    page_dir_delete_slot(page, page_zip, slot_no);
  }
}
