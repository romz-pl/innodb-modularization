#include <innodb/dict_types/id_name_t.h>

#include <ostream>

/** Display an identifier.
@param[in,out]	s	output stream
@param[in]	id_name	SQL identifier (other than table name)
@return the output stream */
std::ostream &operator<<(std::ostream &s, const id_name_t &id_name) {
  const char q = '`';
  const char *c = id_name;
  s << q;
  for (; *c != 0; c++) {
    if (*c == q) {
      s << *c;
    }
    s << *c;
  }
  s << q;
  return (s);
}
