#include <innodb/data_types/dtuple_copy.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_n_v_fields.h>
#include <innodb/data_types/dtuple_create_with_vcol.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dfield_copy.h>

/** Copies a data tuple to another.  This is a shallow copy; if a deep copy
 is desired, dfield_dup() will have to be invoked on each field.
 @return own: copy of tuple */
dtuple_t *dtuple_copy(const dtuple_t *tuple, /*!< in: tuple to copy from */
                      mem_heap_t *heap)      /*!< in: memory heap
                                             where the tuple is created */
{
  ulint n_fields = dtuple_get_n_fields(tuple);
  ulint n_v_fields = dtuple_get_n_v_fields(tuple);
  dtuple_t *new_tuple = dtuple_create_with_vcol(heap, n_fields, n_v_fields);
  ulint i;

  for (i = 0; i < n_fields; i++) {
    dfield_copy(dtuple_get_nth_field(new_tuple, i),
                dtuple_get_nth_field(tuple, i));
  }

  for (i = 0; i < n_v_fields; i++) {
    dfield_copy(dtuple_get_nth_v_field(new_tuple, i),
                dtuple_get_nth_v_field(tuple, i));
  }

  return (new_tuple);
}
