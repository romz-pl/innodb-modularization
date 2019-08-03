#include <innodb/record/rec_set_nth_field_sql_null.h>

#include <innodb/record/rec_get_field_start_offs.h>
#include <innodb/record/data_write_sql_null.h>
#include <innodb/record/rec_get_nth_field_size.h>
#include <innodb/record/rec_set_nth_field_null_bit.h>

/** Sets an old-style record field to SQL null.
 The physical size of the field is not changed. */
void rec_set_nth_field_sql_null(rec_t *rec, /*!< in: record */
                                ulint n)    /*!< in: index of the field */
{
  ulint offset;

  offset = rec_get_field_start_offs(rec, n);

  data_write_sql_null(rec + offset, rec_get_nth_field_size(rec, n));

  rec_set_nth_field_null_bit(rec, n, TRUE);
}
