#include <innodb/print/PrintBuffer.h>

#include <innodb/print/ut_print_buf.h>

std::ostream &PrintBuffer::print(std::ostream &out) const {
  if (m_buf != nullptr) {
    ut_print_buf(out, m_buf, m_len);
  }
  return (out);
}
