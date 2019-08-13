#include <innodb/data_types/dtuple_copy_v_fields.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_get_n_v_fields.h>
#include <innodb/data_types/dfield_copy.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>

/** Copies a data tuple's virtual fields to another. This is a shallow copy;
@param[in,out]	d_tuple		destination tuple
@param[in]	s_tuple		source tuple */
void dtuple_copy_v_fields(dtuple_t *d_tuple, const dtuple_t *s_tuple) {
  ulint n_v_fields = dtuple_get_n_v_fields(d_tuple);
  ut_ad(n_v_fields == dtuple_get_nth_v_field(s_tuple));

  for (ulint i = 0; i < n_v_fields; i++) {
    dfield_copy(dtuple_get_nth_v_field(d_tuple, i),
                dtuple_get_nth_v_field(s_tuple, i));
  }
}
