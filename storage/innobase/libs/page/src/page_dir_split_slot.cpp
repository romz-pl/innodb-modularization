#include <innodb/page/page_dir_split_slot.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_dir_slot_t.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_get_n_owned.h>
#include <innodb/page/page_rec_get_next.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_dir_slot_set_rec.h>
#include <innodb/page/page_dir_slot_set_n_owned.h>
#include <innodb/page/page_dir_add_slot.h>

/** Splits a directory slot which owns too many records. */
void page_dir_split_slot(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be written, or NULL */
    ulint slot_no)            /*!< in: the directory slot */
{
  rec_t *rec;
  page_dir_slot_t *new_slot;
  page_dir_slot_t *prev_slot;
  page_dir_slot_t *slot;
  ulint i;
  ulint n_owned;

  ut_ad(page);
  ut_ad(!page_zip || page_is_comp(page));
  ut_ad(slot_no > 0);

  slot = page_dir_get_nth_slot(page, slot_no);

  n_owned = page_dir_slot_get_n_owned(slot);
  ut_ad(n_owned == PAGE_DIR_SLOT_MAX_N_OWNED + 1);

  /* 1. We loop to find a record approximately in the middle of the
  records owned by the slot. */

  prev_slot = page_dir_get_nth_slot(page, slot_no - 1);
  rec = (rec_t *)page_dir_slot_get_rec(prev_slot);

  for (i = 0; i < n_owned / 2; i++) {
    rec = page_rec_get_next(rec);
  }

  ut_ad(n_owned / 2 >= PAGE_DIR_SLOT_MIN_N_OWNED);

  /* 2. We add one directory slot immediately below the slot to be
  split. */

  page_dir_add_slot(page, page_zip, slot_no - 1);

  /* The added slot is now number slot_no, and the old slot is
  now number slot_no + 1 */

  new_slot = page_dir_get_nth_slot(page, slot_no);
  slot = page_dir_get_nth_slot(page, slot_no + 1);

  /* 3. We store the appropriate values to the new slot. */

  page_dir_slot_set_rec(new_slot, rec);
  page_dir_slot_set_n_owned(new_slot, page_zip, n_owned / 2);

  /* 4. Finally, we update the number of records field of the
  original slot */

  page_dir_slot_set_n_owned(slot, page_zip, n_owned - (n_owned / 2));
}
