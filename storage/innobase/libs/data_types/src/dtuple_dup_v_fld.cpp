#include <innodb/data_types/dtuple_dup_v_fld.h>

#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dfield_dup.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dfield_t.h>

/** Duplicate the virtual field data in a dtuple_t
@param[in,out]		vrow	dtuple contains the virtual fields
@param[in]		heap	heap memory to use */
void dtuple_dup_v_fld(const dtuple_t *vrow, mem_heap_t *heap) {
  for (ulint i = 0; i < vrow->n_v_fields; i++) {
    dfield_t *dfield = dtuple_get_nth_v_field(vrow, i);
    dfield_dup(dfield, heap);
  }
}
