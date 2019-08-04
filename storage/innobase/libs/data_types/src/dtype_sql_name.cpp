#include <innodb/data_types/dtype_sql_name.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/data_types/flags.h>

#include <stdio.h>

/** Returns the type's SQL name (e.g. BIGINT UNSIGNED) from mtype,prtype,len
 @return the SQL type name */
char *dtype_sql_name(unsigned mtype,   /*!< in: mtype */
                     unsigned prtype,  /*!< in: prtype */
                     unsigned len,     /*!< in: len */
                     char *name,       /*!< out: SQL name */
                     unsigned name_sz) /*!< in: size of the name buffer */
{
#define APPEND_UNSIGNED()                                                 \
  do {                                                                    \
    if (prtype & DATA_UNSIGNED) {                                         \
      snprintf(name + strlen(name), name_sz - strlen(name), " UNSIGNED"); \
    }                                                                     \
  } while (0)

  snprintf(name, name_sz, "UNKNOWN");

  switch (mtype) {
    case DATA_INT:
      switch (len) {
        case 1:
          snprintf(name, name_sz, "TINYINT");
          break;
        case 2:
          snprintf(name, name_sz, "SMALLINT");
          break;
        case 3:
          snprintf(name, name_sz, "MEDIUMINT");
          break;
        case 4:
          snprintf(name, name_sz, "INT");
          break;
        case 8:
          snprintf(name, name_sz, "BIGINT");
          break;
      }
      APPEND_UNSIGNED();
      break;
    case DATA_FLOAT:
      snprintf(name, name_sz, "FLOAT");
      APPEND_UNSIGNED();
      break;
    case DATA_DOUBLE:
      snprintf(name, name_sz, "DOUBLE");
      APPEND_UNSIGNED();
      break;
    case DATA_FIXBINARY:
      snprintf(name, name_sz, "BINARY(%u)", len);
      break;
    case DATA_CHAR:
    case DATA_MYSQL:
      snprintf(name, name_sz, "CHAR(%u)", len);
      break;
    case DATA_VARCHAR:
    case DATA_VARMYSQL:
      snprintf(name, name_sz, "VARCHAR(%u)", len);
      break;
    case DATA_BINARY:
      snprintf(name, name_sz, "VARBINARY(%u)", len);
      break;
    case DATA_GEOMETRY:
      snprintf(name, name_sz, "GEOMETRY");
      break;
    case DATA_BLOB:
      switch (len) {
        case 9:
          snprintf(name, name_sz, "TINYBLOB");
          break;
        case 10:
          snprintf(name, name_sz, "BLOB");
          break;
        case 11:
          snprintf(name, name_sz, "MEDIUMBLOB");
          break;
        case 12:
          snprintf(name, name_sz, "LONGBLOB");
          break;
      }
  }

  if (prtype & DATA_NOT_NULL) {
    snprintf(name + strlen(name), name_sz - strlen(name), " NOT NULL");
  }

  return (name);
}

#endif /* !UNIV_HOTBACKUP */


