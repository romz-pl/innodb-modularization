#include <innodb/page/page_dir_delete_slot.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/page/header.h>
#include <innodb/page/page_dir_get_n_slots.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_get_n_owned.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_dir_slot_set_n_owned.h>
#include <innodb/page/page_dir_slot_set_rec.h>
#include <innodb/page/page_header_set_field.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/univ/rec_t.h>

/** Used to delete n slots from the directory. This function updates
 also n_owned fields in the records, so that the first slot after
 the deleted ones inherits the records of the deleted slots. */
void page_dir_delete_slot(
    page_t *page,             /*!< in/out: the index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    ulint slot_no)            /*!< in: slot to be deleted */
{
  page_dir_slot_t *slot;
  ulint n_owned;
  ulint i;
  ulint n_slots;

  ut_ad(!page_zip || page_is_comp(page));
  ut_ad(slot_no > 0);
  ut_ad(slot_no + 1 < page_dir_get_n_slots(page));

  n_slots = page_dir_get_n_slots(page);

  /* 1. Reset the n_owned fields of the slots to be
  deleted */
  slot = page_dir_get_nth_slot(page, slot_no);
  n_owned = page_dir_slot_get_n_owned(slot);
  page_dir_slot_set_n_owned(slot, page_zip, 0);

  /* 2. Update the n_owned value of the first non-deleted slot */

  slot = page_dir_get_nth_slot(page, slot_no + 1);
  page_dir_slot_set_n_owned(slot, page_zip,
                            n_owned + page_dir_slot_get_n_owned(slot));

  /* 3. Destroy the slot by copying slots */
  for (i = slot_no + 1; i < n_slots; i++) {
    rec_t *rec = (rec_t *)page_dir_slot_get_rec(page_dir_get_nth_slot(page, i));
    page_dir_slot_set_rec(page_dir_get_nth_slot(page, i - 1), rec);
  }

  /* 4. Zero out the last slot, which will be removed */
  mach_write_to_2(page_dir_get_nth_slot(page, n_slots - 1), 0);

  /* 5. Update the page header */
  page_header_set_field(page, page_zip, PAGE_N_DIR_SLOTS, n_slots - 1);
}

