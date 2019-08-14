#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_t.h>

#include <ostream>

struct dict_foreign_print {
  dict_foreign_print(std::ostream &out) : m_out(out) {}

  void operator()(const dict_foreign_t *foreign) { m_out << *foreign; }

 private:
  std::ostream &m_out;
};
