#pragma once

#include <innodb/univ/univ.h>

#include <iosfwd>

/** SQL identifier name wrapper for pretty-printing */
class id_name_t {
 public:
  /** Default constructor */
  id_name_t() : m_name() {}
  /** Constructor
  @param[in]	name	identifier to assign */
  explicit id_name_t(const char *name) : m_name(name) {}

  /** Assignment operator
  @param[in]	name	identifier to assign */
  id_name_t &operator=(const char *name) {
    m_name = name;
    return (*this);
  }

  /** Implicit type conversion
  @return the name */
  operator const char *() const { return (m_name); }

  /** Explicit type conversion
  @return the name */
  const char *operator()() const { return (m_name); }

 private:
  /** The name in internal representation */
  const char *m_name;
};


/** Display an identifier.
@param[in,out]	s	output stream
@param[in]	id_name	SQL identifier (other than table name)
@return the output stream */
std::ostream &operator<<(std::ostream &s, const id_name_t &id_name);
