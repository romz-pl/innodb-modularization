#include <innodb/record/rec_offs_print.h>

#ifdef UNIV_DEBUG


/** Print the record offsets.
@param[in]    out         the output stream to which offsets are printed.
@param[in]    offsets     the field offsets of the record.
@return the output stream. */
std::ostream &rec_offs_print(std::ostream &out, const ulint *offsets) {
  ulint n = rec_offs_n_fields(offsets);

  out << "[rec offsets: &offsets[0]=" << (void *)&offsets[0] << ", n=" << n
      << std::endl;
  for (ulint i = 0; i < n; ++i) {
    ulint len;
    ulint field_offset = rec_get_nth_field_offs(offsets, i, &len);
    out << "i=" << i << ", offsets[" << i << "]=" << offsets[i]
        << ", field_offset=" << field_offset << ", len=" << len << std::endl;
  }
  out << "]" << std::endl;
  return (out);
}

#endif /* UNIV_DEBUG */
