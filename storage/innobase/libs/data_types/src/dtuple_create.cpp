#include <innodb/data_types/dtuple_create.h>

#include <innodb/data_types/dtuple_create_with_vcol.h>

/** Creates a data tuple to a memory heap. The default value for number
 of fields used in record comparisons for this tuple is n_fields.
 @return own: created tuple */
dtuple_t *dtuple_create(
    mem_heap_t *heap, /*!< in: memory heap where the tuple
                      is created, DTUPLE_EST_ALLOC(n_fields)
                      bytes will be allocated from this heap */
    ulint n_fields)   /*!< in: number of fields */
{
  return (dtuple_create_with_vcol(heap, n_fields, 0));
}
