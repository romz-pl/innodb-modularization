#pragma once

#include <innodb/univ/univ.h>

#ifdef HAVE_PSI_INTERFACE

/** Define for performance schema registration key */
struct mysql_pfs_key_t {
 public:
  /** Default Constructor */
  mysql_pfs_key_t() { s_count++; }

  /** Constructor */
  mysql_pfs_key_t(unsigned int val) : m_value(val) {}

  /** Retreive the count.
  @return number of keys defined */
  static int get_count() { return s_count; }

  /* Key value. */
  unsigned int m_value;

 private:
  /** To keep count of number of PS keys defined. */
  static unsigned int s_count;
};

#endif /* HAVE_PFS_INTERFACE */
