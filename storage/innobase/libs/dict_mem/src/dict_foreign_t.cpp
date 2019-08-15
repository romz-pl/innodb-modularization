#include <innodb/dict_mem/dict_foreign_t.h>

#include <ostream>

std::ostream &operator<<(std::ostream &out, const dict_foreign_t &foreign) {
  out << "[dict_foreign_t: id='" << foreign.id << "'";

  if (foreign.foreign_table_name != NULL) {
    out << ",for: '" << foreign.foreign_table_name << "'";
  }

  out << "]";
  return (out);
}
