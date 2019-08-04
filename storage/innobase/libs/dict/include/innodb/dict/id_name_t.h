#pragma once

#include <innodb/univ/univ.h>

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
