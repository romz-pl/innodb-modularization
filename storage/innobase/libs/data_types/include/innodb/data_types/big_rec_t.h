#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>
#include <innodb/data_types/big_rec_field_t.h>

/** Storage format for overflow data in a big record, that is, a
clustered index record which needs external storage of data fields */
struct big_rec_t {
  mem_heap_t *heap;        /*!< memory heap from which
                           allocated */
  const ulint capacity;    /*!< fields array size */
  ulint n_fields;          /*!< number of stored fields */
  big_rec_field_t *fields; /*!< stored fields */

  /** Constructor.
  @param[in]	max	the capacity of the array of fields. */
  explicit big_rec_t(const ulint max)
      : heap(0), capacity(max), n_fields(0), fields(0) {}

  /** Append one big_rec_field_t object to the end of array of fields */
  void append(const big_rec_field_t &field) {
    ut_ad(n_fields < capacity);
    fields[n_fields] = field;
    n_fields++;
  }

  /** Allocate a big_rec_t object in the given memory heap, and for
  storing n_fld number of fields.
  @param[in]	heap	memory heap in which this object is allocated
  @param[in]	n_fld	maximum number of fields that can be stored in
                  this object
  @return the allocated object */
  static big_rec_t *alloc(mem_heap_t *heap, ulint n_fld);

  /** Print the current object into the given output stream.
  @param[in]	out	the output stream.
  @return	the ouput stream. */
  std::ostream &print(std::ostream &out) const;
};

/** Overloading the global output operator to easily print the given
big_rec_t object into the given output stream.
@param[in]	out	the output stream
@param[in]	obj	the given object to print.
@return the output stream. */
inline std::ostream &operator<<(std::ostream &out, const big_rec_t &obj) {
  return (obj.print(out));
}
