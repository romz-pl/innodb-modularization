#include <innodb/data_types/dfield_set_data.h>

#include <innodb/data_types/dfield_t.h>

/** Sets pointer to the data and length in a field. */
void dfield_set_data(dfield_t *field,  /*!< in: field */
                     const void *data, /*!< in: data */
                     ulint len)        /*!< in: length or UNIV_SQL_NULL */
{
  ut_ad(field);

#ifdef UNIV_VALGRIND_DEBUG
  if (len != UNIV_SQL_NULL) UNIV_MEM_ASSERT_RW(data, len);
#endif /* UNIV_VALGRIND_DEBUG */
  field->data = (void *)data;
  field->ext = 0;
  field->len = static_cast<unsigned int>(len);
}
