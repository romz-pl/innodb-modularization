#include <innodb/lock_priv/lock_rec_t.h>

#include <ostream>

/** Print the record lock into the given output stream
@param[in,out]	out	the output stream
@return the given output stream. */
std::ostream &lock_rec_t::print(std::ostream &out) const {
  out << "[lock_rec_t: space=" << space << ", page_no=" << page_no
      << ", n_bits=" << n_bits << "]";
  return (out);
}


std::ostream &operator<<(std::ostream &out, const lock_rec_t &lock) {
  return (lock.print(out));
}
