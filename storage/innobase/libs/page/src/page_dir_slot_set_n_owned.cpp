#include <innodb/page/page_dir_slot_set_n_owned.h>

#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_rec_is_comp.h>
#include <innodb/record/rec_set_n_owned_new.h>
#include <innodb/record/rec_set_n_owned_old.h>
#include <innodb/assert/assert.h>

/** This is used to set the owned records field of a directory slot. */
void page_dir_slot_set_n_owned(
    page_dir_slot_t *slot,    /*!< in/out: directory slot */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    ulint n)                  /*!< in: number of records owned by the slot */
{
  rec_t *rec = (rec_t *)page_dir_slot_get_rec(slot);
  if (page_rec_is_comp(slot)) {
    rec_set_n_owned_new(rec, page_zip, n);
  } else {
    ut_ad(!page_zip);
    rec_set_n_owned_old(rec, n);
  }
}
