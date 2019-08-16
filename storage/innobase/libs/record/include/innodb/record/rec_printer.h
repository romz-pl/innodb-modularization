#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG
/** Pretty-printer of records and tuples */
class rec_printer : public std::ostringstream {
 public:
  /** Construct a pretty-printed record.
  @param rec	record with header
  @param offsets	rec_get_offsets(rec, ...) */
  rec_printer(const rec_t *rec, const ulint *offsets) : std::ostringstream() {
    rec_print(*this, rec, rec_get_info_bits(rec, rec_offs_comp(offsets)),
              offsets);
  }

  /** Construct a pretty-printed record.
  @param rec record, possibly lacking header
  @param info rec_get_info_bits(rec)
  @param offsets rec_get_offsets(rec, ...) */
  rec_printer(const rec_t *rec, ulint info, const ulint *offsets)
      : std::ostringstream() {
    rec_print(*this, rec, info, offsets);
  }

  /** Construct a pretty-printed tuple.
  @param tuple	data tuple */
  rec_printer(const dtuple_t *tuple) : std::ostringstream() {
    dtuple_print(*this, tuple);
  }

  /** Construct a pretty-printed tuple.
  @param field	array of data tuple fields
  @param n	number of fields */
  rec_printer(const dfield_t *field, ulint n) : std::ostringstream() {
    dfield_print(*this, field, n);
  }

  /** Destructor */
  virtual ~rec_printer() {}

 private:
  /** Copy constructor */
  rec_printer(const rec_printer &other);
  /** Assignment operator */
  rec_printer &operator=(const rec_printer &other);
};
#endif /* UNIV_DEBUG */
