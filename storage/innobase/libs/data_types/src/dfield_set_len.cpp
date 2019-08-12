#include <innodb/data_types/dfield_set_len.h>

#include <innodb/data_types/dfield_t.h>

/** Sets length in a field. */
void dfield_set_len(dfield_t *field, /*!< in: field */
                    ulint len)       /*!< in: length or UNIV_SQL_NULL */
{
  ut_ad(field);
#ifdef UNIV_VALGRIND_DEBUG
  if (len != UNIV_SQL_NULL) UNIV_MEM_ASSERT_RW(field->data, len);
#endif /* UNIV_VALGRIND_DEBUG */

  field->ext = 0;
  field->len = static_cast<unsigned int>(len);
}
