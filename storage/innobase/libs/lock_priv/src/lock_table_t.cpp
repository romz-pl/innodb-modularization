#include <innodb/lock_priv/lock_table_t.h>

#include <innodb/dict_mem/dict_table_t.h>

#include <ostream>


/** Print the table lock into the given output stream
@param[in,out]	out	the output stream
@return the given output stream. */
std::ostream &lock_table_t::print(std::ostream &out) const {
  out << "[lock_table_t: name=" << table->name << "]";
  return (out);
}

/** The global output operator is overloaded to conveniently
print the lock_table_t object into the given output stream.
@param[in,out]	out	the output stream
@param[in]	lock	the table lock
@return the given output stream */
std::ostream &operator<<(std::ostream &out, const lock_table_t &lock) {
  return (lock.print(out));
}
