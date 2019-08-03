#include <innodb/record/rec_get_heap_no_new.h>

#include <innodb/record/rec_get_bit_field_2.h>
#include <innodb/record/flag.h>

/** The following function is used to get the order number
 of a new-style record in the heap of the index page.
 @return heap order number */
ulint rec_get_heap_no_new(const rec_t *rec) /*!< in: physical record */
{
  return (rec_get_bit_field_2(rec, REC_NEW_HEAP_NO, REC_HEAP_NO_MASK,
                              REC_HEAP_NO_SHIFT));
}
