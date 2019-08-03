#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

#include <iosfwd>

/** Wrapper for pretty-printing a record */
struct rec_offsets_print {
  /** Constructor */
  rec_offsets_print(const rec_t *rec, const ulint *offsets)
      : m_rec(rec), m_offsets(offsets) {}

  /** Record */
  const rec_t *m_rec;
  /** Offsets to each field */
  const ulint *m_offsets;
};

/** Display a record.
@param[in,out]	o	output stream
@param[in]	r	record to display
@return	the output stream */
std::ostream &operator<<(std::ostream &o, const rec_offsets_print &r);
