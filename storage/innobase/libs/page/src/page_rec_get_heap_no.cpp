#include <innodb/page/page_rec_get_heap_no.h>

#include <innodb/page/page_rec_is_comp.h>

/** Returns the heap number of a record.
 @return heap number */
ulint page_rec_get_heap_no(const rec_t *rec) /*!< in: the physical record */
{
  if (page_rec_is_comp(rec)) {
    return (rec_get_heap_no_new(rec));
  } else {
    return (rec_get_heap_no_old(rec));
  }
}
