#include <innodb/record/rec_index_print.h>

#include <innodb/record/rec_print.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/memory/mem_heap_free.h>

#include <ostream>

/** Display a record.
@param[in,out]	o	output stream
@param[in]	r	record to display
@return	the output stream */
std::ostream &operator<<(std::ostream &o, const rec_index_print &r) {
  mem_heap_t *heap = NULL;
  ulint *offsets =
      rec_get_offsets(r.m_rec, r.m_index, NULL, ULINT_UNDEFINED, &heap);
  rec_print(o, r.m_rec, rec_get_info_bits(r.m_rec, rec_offs_comp(offsets)),
            offsets);
  mem_heap_free(heap);
  return (o);
}
