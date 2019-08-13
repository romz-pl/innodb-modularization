#include <innodb/data_types/big_rec_t.h>

#include <innodb/memory/mem_heap_alloc.h>

/** Allocate a big_rec_t object in the given memory heap, and for storing
n_fld number of fields.
@param[in]	heap	memory heap in which this object is allocated
@param[in]	n_fld	maximum number of fields that can be stored in
                        this object

@return the allocated object */
big_rec_t *big_rec_t::alloc(mem_heap_t *heap, ulint n_fld) {
  big_rec_t *rec =
      static_cast<big_rec_t *>(mem_heap_alloc(heap, sizeof(big_rec_t)));

  new (rec) big_rec_t(n_fld);

  rec->heap = heap;
  rec->fields = static_cast<big_rec_field_t *>(
      mem_heap_alloc(heap, n_fld * sizeof(big_rec_field_t)));

  rec->n_fields = 0;
  return (rec);
}

