#include <innodb/record/rec_print_comp.h>

#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_default.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/record/flag.h>

#include <innodb/print/ut_print_buf.h>

#ifndef UNIV_HOTBACKUP

/** Prints a physical record in ROW_FORMAT=COMPACT.  Ignores the
 record header. */
void rec_print_comp(
    FILE *file,           /*!< in: file where to print */
    const rec_t *rec,     /*!< in: physical record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ulint i;

  for (i = 0; i < rec_offs_n_fields(offsets); i++) {
    const byte *data = nullptr;
    ulint len;

    if (rec_offs_nth_default(offsets, i)) {
      len = UNIV_SQL_ADD_COL_DEFAULT;
    } else {
      data = rec_get_nth_field(rec, offsets, i, &len);
    }

    fprintf(file, " %lu:", (ulong)i);

    switch (len) {
      case UNIV_SQL_NULL:
        fputs(" SQL NULL", file);
        break;
      case UNIV_SQL_ADD_COL_DEFAULT:
        fputs(" SQL DEFAULT", file);
        break;
      default:
        if (len <= 30) {
          ut_print_buf(file, data, len);
        } else if (rec_offs_nth_extern(offsets, i)) {
          ut_print_buf(file, data, 30);
          fprintf(file, " (total %lu bytes, external)", (ulong)len);
          ut_print_buf(file, data + len - BTR_EXTERN_FIELD_REF_SIZE,
                       BTR_EXTERN_FIELD_REF_SIZE);
        } else {
          ut_print_buf(file, data, 30);

          fprintf(file, " (total %lu bytes)", (ulong)len);
        }
    }
    putc(';', file);
    putc('\n', file);
  }
}


#endif
