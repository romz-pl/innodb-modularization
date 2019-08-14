#pragma once

#include <innodb/univ/univ.h>

#include <iosfwd>

/** Table name wrapper for pretty-printing */
struct table_name_t {
  /** The name in internal representation */
  char *m_name;
};


/** Display a table name.
@param[in,out]	s		output stream
@param[in]	table_name	table name
@return the output stream */
std::ostream &operator<<(std::ostream &s, const table_name_t &table_name);
