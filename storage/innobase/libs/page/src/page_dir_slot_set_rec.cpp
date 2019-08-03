#include <innodb/page/page_dir_slot_set_rec.h>

#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_offset.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

/** This is used to set the record offset in a directory slot. */
void page_dir_slot_set_rec(page_dir_slot_t *slot, /*!< in: directory slot */
                           rec_t *rec)            /*!< in: record on the page */
{
  ut_ad(page_rec_check(rec));

  mach_write_to_2(slot, page_offset(rec));
}
