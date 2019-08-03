#include <innodb/page/page_dir_slot_get_rec.h>

#include <innodb/page/page_align.h>
#include <innodb/machine/data.h>

/** Gets the record pointed to by a directory slot.
 @return pointer to record */
const rec_t *page_dir_slot_get_rec(
    const page_dir_slot_t *slot) /*!< in: directory slot */
{
  return (page_align(slot) + mach_read_from_2(slot));
}
