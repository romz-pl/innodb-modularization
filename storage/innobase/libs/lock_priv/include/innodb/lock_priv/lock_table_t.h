#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>

#include <iosfwd>

struct dict_table_t;
struct lock_t;

/** A table lock */
struct lock_table_t {
  dict_table_t *table; /*!< database table in dictionary
                       cache */
  UT_LIST_NODE_T(lock_t)
  locks; /*!< list of locks on the same
         table */
  /** Print the table lock into the given output stream
  @param[in,out]	out	the output stream
  @return the given output stream. */
  std::ostream &print(std::ostream &out) const;
};


/** The global output operator is overloaded to conveniently
print the lock_table_t object into the given output stream.
@param[in,out]	out	the output stream
@param[in]	lock	the table lock
@return the given output stream */
std::ostream &operator<<(std::ostream &out, const lock_table_t &lock);
