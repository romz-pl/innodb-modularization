#include <innodb/data_types/dtuple_create_from_mem.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/data_types/DTUPLE_EST_ALLOC.h>

/** Creates a data tuple from an already allocated chunk of memory.
The size of the chunk must be at least DTUPLE_EST_ALLOC(n_fields).
The default value for number of fields used in record comparisons
for this tuple is n_fields.
@param[in,out]	buf		buffer to use
@param[in]	buf_size	buffer size
@param[in]	n_fields	number of field
@param[in]	n_v_fields	number of fields on virtual columns
@return created tuple (inside buf) */
dtuple_t *dtuple_create_from_mem(void *buf, ulint buf_size, ulint n_fields,
                                 ulint n_v_fields) {
  dtuple_t *tuple;
  ulint n_t_fields = n_fields + n_v_fields;

  ut_ad(buf != NULL);
  ut_a(buf_size >= DTUPLE_EST_ALLOC(n_t_fields));

  tuple = (dtuple_t *)buf;
  tuple->info_bits = 0;
  tuple->n_fields = n_fields;
  tuple->n_v_fields = n_v_fields;
  tuple->n_fields_cmp = n_fields;
  tuple->fields = (dfield_t *)&tuple[1];
  if (n_v_fields > 0) {
    tuple->v_fields = &tuple->fields[n_fields];
  } else {
    tuple->v_fields = NULL;
  }

#ifdef UNIV_DEBUG
  tuple->magic_n = DATA_TUPLE_MAGIC_N;

  { /* In the debug version, initialize fields to an error value */
    ulint i;

    for (i = 0; i < n_t_fields; i++) {
      dfield_t *field;

      if (i >= n_fields) {
        field = dtuple_get_nth_v_field(tuple, i - n_fields);
      } else {
        field = dtuple_get_nth_field(tuple, i);
      }

      dfield_set_len(field, UNIV_SQL_NULL);
      field->data = &data_error;
      dfield_get_type(field)->mtype = DATA_ERROR;
      dfield_get_type(field)->prtype = DATA_ERROR;
    }
  }
#endif
  UNIV_MEM_ASSERT_W(tuple->fields, n_t_fields * sizeof *tuple->fields);
  UNIV_MEM_INVALID(tuple->fields, n_t_fields * sizeof *tuple->fields);
  return (tuple);
}
