#include <innodb/record/rec_get_nth_field_size.h>

#include <innodb/record/rec_get_field_start_offs.h>
#include <innodb/assert/assert.h>

/** Gets the physical size of an old-style field.
 Also an SQL null may have a field of size > 0,
 if the data type is of a fixed size.
 @return field size in bytes */
ulint rec_get_nth_field_size(const rec_t *rec, /*!< in: record */
                             ulint n)          /*!< in: index of the field */
{
  ulint os;
  ulint next_os;

  os = rec_get_field_start_offs(rec, n);
  next_os = rec_get_field_start_offs(rec, n + 1);

  ut_ad(next_os - os < UNIV_PAGE_SIZE);

  return (next_os - os);
}
