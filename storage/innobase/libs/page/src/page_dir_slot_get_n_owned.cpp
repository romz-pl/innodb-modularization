#include <innodb/page/page_dir_slot_get_n_owned.h>

#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_rec_is_comp.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_n_owned_old.h>

/** Gets the number of records owned by a directory slot.
 @return number of records */
ulint page_dir_slot_get_n_owned(
    const page_dir_slot_t *slot) /*!< in: page directory slot */
{
  const rec_t *rec = page_dir_slot_get_rec(slot);
  if (page_rec_is_comp(slot)) {
    return (rec_get_n_owned_new(rec));
  } else {
    return (rec_get_n_owned_old(rec));
  }
}
