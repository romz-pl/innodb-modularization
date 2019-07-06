#pragma once

#include <innodb/univ/univ.h>

struct PrintBuffer {
  PrintBuffer(const void *buf, ulint len) : m_buf(buf), m_len(len) {}

  std::ostream &print(std::ostream &out) const;

 private:
  const void *m_buf;
  ulint m_len;
};

inline std::ostream &operator<<(std::ostream &out, const PrintBuffer &obj) {
  return (obj.print(out));
}
