#include <innodb/data_types/dtuple_create_with_vcol.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/DTUPLE_EST_ALLOC.h>
#include <innodb/data_types/dtuple_create_from_mem.h>
#include <innodb/memory/mem_heap_zalloc.h>

/** Creates a data tuple with virtual columns to a memory heap.
@param[in]	heap		memory heap where the tuple is created
@param[in]	n_fields	number of fields
@param[in]	n_v_fields	number of fields on virtual col
@return own: created tuple */
dtuple_t *dtuple_create_with_vcol(mem_heap_t *heap, ulint n_fields,
                                  ulint n_v_fields) {
  void *buf;
  ulint buf_size;
  dtuple_t *tuple;

  ut_ad(heap);

  buf_size = DTUPLE_EST_ALLOC(n_fields + n_v_fields);
  buf = mem_heap_zalloc(heap, buf_size);

  tuple = dtuple_create_from_mem(buf, buf_size, n_fields, n_v_fields);

#ifdef UNIV_DEBUG
  tuple->m_heap = heap;
#endif /* UNIV_DEBUG */

  return (tuple);
}
