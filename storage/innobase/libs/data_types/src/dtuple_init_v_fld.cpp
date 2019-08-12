#include <innodb/data_types/dtuple_init_v_fld.h>

#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_set_len.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/flags.h>

/** Initialize the virtual field data in a dtuple_t
@param[in,out]		vrow	dtuple contains the virtual fields */
void dtuple_init_v_fld(const dtuple_t *vrow) {
  for (ulint i = 0; i < vrow->n_v_fields; i++) {
    dfield_t *dfield = dtuple_get_nth_v_field(vrow, i);
    dfield_get_type(dfield)->mtype = DATA_MISSING;
    dfield_set_len(dfield, UNIV_SQL_NULL);
  }
}
