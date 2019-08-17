#include <innodb/page/page_dir_add_slot.h>

#include <innodb/page/page_dir_set_n_slots.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_t.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_dir_get_n_slots.h>


/** Used to add n slots to the directory. Does not set the record pointers
 in the added slots or update n_owned values: this is the responsibility
 of the caller. */
void page_dir_add_slot(
    page_t *page,             /*!< in/out: the index page */
    page_zip_des_t *page_zip, /*!< in/out: comprssed page, or NULL */
    ulint start)              /*!< in: the slot above which the new slots
                              are added */
{
  page_dir_slot_t *slot;
  ulint n_slots;

  n_slots = page_dir_get_n_slots(page);

  ut_ad(start < n_slots - 1);

  /* Update the page header */
  page_dir_set_n_slots(page, page_zip, n_slots + 1);

  /* Move slots up */
  slot = page_dir_get_nth_slot(page, n_slots);
  memmove(slot, slot + PAGE_DIR_SLOT_SIZE,
          (n_slots - 1 - start) * PAGE_DIR_SLOT_SIZE);
}
