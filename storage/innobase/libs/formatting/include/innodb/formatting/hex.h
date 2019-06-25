#pragma once

#include <innodb/univ/univ.h>
#include <ostream>
#include <ios>



namespace ib {


/** This is a wrapper class, used to print any unsigned integer type
in hexadecimal format.  The main purpose of this data type is to
overload the global operator<<, so that we can print the given
wrapper value in hex. */
struct hex {
  explicit hex(uintmax_t t) : m_val(t) {}
  const uintmax_t m_val;
};

/** This is an overload of the global operator<< for the user defined type
ib::hex.  The unsigned value held in the ib::hex wrapper class will be printed
into the given output stream in hexadecimal format.
@param[in,out]	lhs	the output stream into which rhs is written.
@param[in]	rhs	the object to be written into lhs.
@retval	reference to the output stream. */
inline std::ostream &operator<<(std::ostream &lhs, const hex &rhs) {
  std::ios_base::fmtflags ff = lhs.flags();
  lhs << std::showbase << std::hex << rhs.m_val;
  lhs.setf(ff);
  return (lhs);
}








}  // namespace ib
