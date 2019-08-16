#include <innodb/record/rec_copy_prefix_to_dtuple.h>

#include <innodb/data_types/dfield_set_data.h>
#include <innodb/data_types/dfield_set_null.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_set_info_bits.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/memory/mem_heap_dup.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_get_nth_field_instant.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_offs_init.h>


/** Copies the first n fields of a physical record to a data tuple. The fields
 are copied to the memory heap. */
void rec_copy_prefix_to_dtuple(
    dtuple_t *tuple,           /*!< out: data tuple */
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n_fields,            /*!< in: number of fields
                               to copy */
    mem_heap_t *heap)          /*!< in: memory heap */
{
  ulint i;
  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  ulint *offsets = offsets_;
  rec_offs_init(offsets_);

  offsets = rec_get_offsets(rec, index, offsets, n_fields, &heap);

  ut_ad(rec_validate(rec, offsets));
  ut_ad(dtuple_check_typed(tuple));

  dtuple_set_info_bits(
      tuple, rec_get_info_bits(rec, dict_table_is_comp(index->table)));

  for (i = 0; i < n_fields; i++) {
    dfield_t *field;
    const byte *data;
    ulint len;

    field = dtuple_get_nth_field(tuple, i);
    data = rec_get_nth_field_instant(rec, offsets, i, index, &len);

    if (len != UNIV_SQL_NULL) {
      dfield_set_data(field, mem_heap_dup(heap, data, len), len);
      ut_ad(!rec_offs_nth_extern(offsets, i));
    } else {
      dfield_set_null(field);
    }
  }
}
