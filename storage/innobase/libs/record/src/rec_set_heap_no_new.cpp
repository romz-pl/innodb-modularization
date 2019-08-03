#include <innodb/record/rec_set_heap_no_new.h>

#include <innodb/record/rec_set_bit_field_2.h>
#include <innodb/record/flag.h>

/** The following function is used to set the heap number
 field in a new-style record. */
void rec_set_heap_no_new(rec_t *rec,    /*!< in/out: physical record */
                         ulint heap_no) /*!< in: the heap number */
{
  rec_set_bit_field_2(rec, heap_no, REC_NEW_HEAP_NO, REC_HEAP_NO_MASK,
                      REC_HEAP_NO_SHIFT);
}
