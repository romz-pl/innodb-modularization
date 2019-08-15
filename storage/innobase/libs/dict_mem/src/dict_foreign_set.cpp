#include <innodb/dict_mem/dict_foreign_set.h>

#include <innodb/dict_mem/dict_foreign_print.h>

#include <ostream>
#include <algorithm>

std::ostream &operator<<(std::ostream &out, const dict_foreign_set &fk_set) {
  out << "[dict_foreign_set:";
  std::for_each(fk_set.begin(), fk_set.end(), dict_foreign_print(out));
  out << "]" << std::endl;
  return (out);
}
