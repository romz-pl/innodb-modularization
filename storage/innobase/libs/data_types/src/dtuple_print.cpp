#include <innodb/data_types/dtuple_print.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_info_bits.h>
#include <innodb/data_types/dfield_print.h>
#include <innodb/data_types/dfield_print_raw.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_validate.h>

#include <ostream>

/** Print the contents of a tuple.
@param[out]	o	output stream
@param[in]	tuple	data tuple */
void dtuple_print(std::ostream &o, const dtuple_t *tuple) {
  const ulint n = dtuple_get_n_fields(tuple);

  o << "TUPLE (info_bits=" << dtuple_get_info_bits(tuple) << ", " << n
    << " fields): {";

  dfield_print(o, tuple->fields, n);

  o << "}";
}


/** The following function prints the contents of a tuple. */
void dtuple_print(FILE *f,               /*!< in: output stream */
                  const dtuple_t *tuple) /*!< in: tuple */
{
  ulint n_fields;
  ulint i;

  n_fields = dtuple_get_n_fields(tuple);

  fprintf(f, "DATA TUPLE: %lu fields;\n", (ulong)n_fields);

  for (i = 0; i < n_fields; i++) {
    fprintf(f, " %lu:", (ulong)i);

    dfield_print_raw(f, dtuple_get_nth_field(tuple, i));

    putc(';', f);
    putc('\n', f);
  }

  ut_ad(dtuple_validate(tuple));
}

