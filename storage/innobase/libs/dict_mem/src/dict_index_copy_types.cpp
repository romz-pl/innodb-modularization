#include <innodb/dict_mem/dict_index_copy_types.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_is_ibuf.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dtuple_set_types_binary.h>
#include <innodb/data_types/dtuple_get_nth_field.h>

/** Copies types of fields contained in index to tuple. */
void dict_index_copy_types(dtuple_t *tuple,           /*!< in/out: data tuple */
                           const dict_index_t *index, /*!< in: index */
                           ulint n_fields)            /*!< in: number of
                                                      field types to copy */
{
  ulint i;

  if (dict_index_is_ibuf(index)) {
    dtuple_set_types_binary(tuple, n_fields);

    return;
  }

  for (i = 0; i < n_fields; i++) {
    const dict_field_t *ifield;
    dtype_t *dfield_type;

    ifield = index->get_field(i);
    dfield_type = dfield_get_type(dtuple_get_nth_field(tuple, i));
    ifield->col->copy_type(dfield_type);
    if (dict_index_is_spatial(index) &&
        DATA_GEOMETRY_MTYPE(dfield_type->mtype)) {
      dfield_type->prtype |= DATA_GIS_MBR;
    }
  }
}

