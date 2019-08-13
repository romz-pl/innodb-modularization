#include <innodb/data_types/dfield_print_raw.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/print/ut_print_buf.h>

#ifndef UNIV_HOTBACKUP

/** Print a dfield value using ut_print_buf. */
void dfield_print_raw(FILE *f,                /*!< in: output stream */
                             const dfield_t *dfield) /*!< in: dfield */
{
  ulint len = dfield_get_len(dfield);
  if (!dfield_is_null(dfield)) {
    ulint print_len = std::min(len, static_cast<ulint>(1000));
    ut_print_buf(f, dfield_get_data(dfield), print_len);
    if (len != print_len) {
      fprintf(f, "(total %lu bytes%s)", (ulong)len,
              dfield_is_ext(dfield) ? ", external" : "");
    }
  } else {
    fputs(" SQL NULL", f);
  }
}


#endif
