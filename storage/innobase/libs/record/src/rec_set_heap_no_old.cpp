#include <innodb/record/rec_set_heap_no_old.h>

#include <innodb/record/rec_set_bit_field_2.h>
#include <innodb/record/flag.h>

/** The following function is used to set the heap number
 field in an old-style record. */
void rec_set_heap_no_old(rec_t *rec,    /*!< in: physical record */
                         ulint heap_no) /*!< in: the heap number */
{
  rec_set_bit_field_2(rec, heap_no, REC_OLD_HEAP_NO, REC_HEAP_NO_MASK,
                      REC_HEAP_NO_SHIFT);
}
