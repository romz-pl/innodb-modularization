#include <innodb/data_types/dtuple_coll_eq.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_nth_field.h>


int cmp_dfield_dfield(const dfield_t *dfield1, const dfield_t *dfield2,
                      bool is_asc);

/** Compare two data tuples.
@param[in] tuple1 first data tuple
@param[in] tuple2 second data tuple
@return whether tuple1 == tuple2 */
bool dtuple_coll_eq(const dtuple_t *tuple1, const dtuple_t *tuple2) {
  ulint n_fields;
  ulint i;
  int cmp;

  ut_ad(tuple1 != NULL);
  ut_ad(tuple2 != NULL);
  ut_ad(tuple1->magic_n == DATA_TUPLE_MAGIC_N);
  ut_ad(tuple2->magic_n == DATA_TUPLE_MAGIC_N);
  ut_ad(dtuple_check_typed(tuple1));
  ut_ad(dtuple_check_typed(tuple2));

  n_fields = dtuple_get_n_fields(tuple1);

  cmp = (int)n_fields - (int)dtuple_get_n_fields(tuple2);

  for (i = 0; cmp == 0 && i < n_fields; i++) {
    const dfield_t *field1 = dtuple_get_nth_field(tuple1, i);
    const dfield_t *field2 = dtuple_get_nth_field(tuple2, i);
    /* Equality comparison does not care about ASC/DESC. */
    cmp = cmp_dfield_dfield(field1, field2, true);
  }

  return (cmp == 0);
}


#endif
