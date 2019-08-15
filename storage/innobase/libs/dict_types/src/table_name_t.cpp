#include <innodb/dict_types/table_name_t.h>

#include <ostream>

struct trx_t;
std::string ut_get_name(const trx_t *trx, const char *name);

/** Display a table name.
@param[in,out]	s		output stream
@param[in]	table_name	table name
@return the output stream */
std::ostream &operator<<(std::ostream &s, const table_name_t &table_name) {
#ifndef UNIV_HOTBACKUP
  return (s << ut_get_name(NULL, table_name.m_name));
#else  /* !UNIV_HOTBACKUP */
  return (s << table_name.m_name);
#endif /* !UNIV_HOTBACKUP */
}
