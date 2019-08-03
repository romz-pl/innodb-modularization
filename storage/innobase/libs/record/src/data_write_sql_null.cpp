#include <innodb/record/data_write_sql_null.h>

#include <string.h>

/** Writes an SQL null field full of zeros. */
void data_write_sql_null(byte *data, /*!< in: pointer to a buffer of size len */
                         ulint len)  /*!< in: SQL null size in bytes */
{
  memset(data, 0, len);
}
