#include <innodb/data_types/dfield_write_mbr.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/machine/data.h>
#include <innodb/data_types/flags.h>

/** Sets pointer to the data and length in a field. */
void dfield_write_mbr(dfield_t *field,   /*!< in: field */
                      const double *mbr) /*!< in: data */
{
  ut_ad(field);

#ifdef UNIV_VALGRIND_DEBUG
  if (len != UNIV_SQL_NULL) UNIV_MEM_ASSERT_RW(data, len);
#endif /* UNIV_VALGRIND_DEBUG */
  field->ext = 0;

  for (int i = 0; i < SPDIMS * 2; i++) {
    mach_double_write(static_cast<byte *>(field->data) + i * sizeof(double),
                      mbr[i]);
  }

  field->len = DATA_MBR_LEN;
}
