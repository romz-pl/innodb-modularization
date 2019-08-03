#include <innodb/page/page_dir_slot_check.h>

#include <innodb/assert/assert.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_get_n_slots.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_n_owned_old.h>

/** Used to check the consistency of a directory slot.
 @return true if succeed */
ibool page_dir_slot_check(const page_dir_slot_t *slot) /*!< in: slot */
{
  const page_t *page;
  ulint n_slots;
  ulint n_owned;

  ut_a(slot);

  page = page_align(slot);

  n_slots = page_dir_get_n_slots(page);

  ut_a(slot <= page_dir_get_nth_slot(page, 0));
  ut_a(slot >= page_dir_get_nth_slot(page, n_slots - 1));

  ut_a(page_rec_check(page_dir_slot_get_rec(slot)));

  if (page_is_comp(page)) {
    n_owned = rec_get_n_owned_new(page_dir_slot_get_rec(slot));
  } else {
    n_owned = rec_get_n_owned_old(page_dir_slot_get_rec(slot));
  }

  if (slot == page_dir_get_nth_slot(page, 0)) {
    ut_a(n_owned == 1);
  } else if (slot == page_dir_get_nth_slot(page, n_slots - 1)) {
    ut_a(n_owned >= 1);
    ut_a(n_owned <= PAGE_DIR_SLOT_MAX_N_OWNED);
  } else {
    ut_a(n_owned >= PAGE_DIR_SLOT_MIN_N_OWNED);
    ut_a(n_owned <= PAGE_DIR_SLOT_MAX_N_OWNED);
  }

  return (TRUE);
}
