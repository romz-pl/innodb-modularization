#include <innodb/dict_mem/dict_foreign_error_report_low.h>

#include <innodb/print/ut_print_timestamp.h>

/** Report an error in a foreign key definition. */
void dict_foreign_error_report_low(
    FILE *file,       /*!< in: output stream */
    const char *name) /*!< in: table name */
{
  rewind(file);
  ut_print_timestamp(file);
  fprintf(file, " Error in foreign key constraint of table %s:\n", name);
}
