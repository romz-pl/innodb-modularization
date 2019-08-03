#include <innodb/record/rec_offsets_print.h>

#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_print.h>

#include <ostream>

/** Display a record.
@param[in,out]	o	output stream
@param[in]	r	record to display
@return	the output stream */
std::ostream &operator<<(std::ostream &o, const rec_offsets_print &r) {
  rec_print(o, r.m_rec, rec_get_info_bits(r.m_rec, rec_offs_comp(r.m_offsets)),
            r.m_offsets);
  return (o);
}
