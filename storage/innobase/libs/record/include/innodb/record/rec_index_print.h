#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#include <iosfwd>

struct dict_index_t;


/** Wrapper for pretty-printing a record */
struct rec_index_print {
  /** Constructor */
  rec_index_print(const rec_t *rec, const dict_index_t *index)
      : m_rec(rec), m_index(index) {}

  /** Record */
  const rec_t *m_rec;
  /** Index */
  const dict_index_t *m_index;
};

/** Display a record.
@param[in,out]	o	output stream
@param[in]	r	record to display
@return	the output stream */
std::ostream &operator<<(std::ostream &o, const rec_index_print &r);
