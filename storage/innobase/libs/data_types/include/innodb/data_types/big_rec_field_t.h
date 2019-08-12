#pragma once

#include <innodb/univ/univ.h>

#include <iosfwd>

/** A slot for a field in a big rec vector */
struct big_rec_field_t {
  /** Constructor.
  @param[in]	field_no_	the field number
  @param[in]	len_		the data length
  @param[in]	data_		the data */
  big_rec_field_t(ulint field_no_, ulint len_, void *data_)
      : field_no(field_no_),
        len(len_),
        data(data_),
        ext_in_old(false),
        ext_in_new(false) {}

  byte *ptr() const { return (static_cast<byte *>(data)); }

  ulint field_no; /*!< field number in record */
  ulint len;      /*!< stored data length, in bytes */
  void *data;     /*!< stored data */

  /** If true, this field was stored externally in the old row.
  If false, this field was stored inline in the old row.*/
  bool ext_in_old;

  /** If true, this field is stored externally in the new row.
  If false, this field is stored inline in the new row.*/
  bool ext_in_new;

  /** Print the big_rec_field_t object into the given output stream.
  @param[in]	out	the output stream.
  @return	the ouput stream. */
  std::ostream &print(std::ostream &out) const;
};

/** Overloading the global output operator to easily print the given
big_rec_field_t object into the given output stream.
@param[in]	out	the output stream
@param[in]	obj	the given object to print.
@return the output stream. */
inline std::ostream &operator<<(std::ostream &out, const big_rec_field_t &obj) {
  return (obj.print(out));
}
