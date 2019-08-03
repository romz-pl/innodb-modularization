#include <innodb/record/rec_print_mbr_rec.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_default.h>
#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_print_mbr_old.h>
#include <innodb/record/rec_validate.h>

#ifndef UNIV_HOTBACKUP

/** Prints a spatial index record. */
void rec_print_mbr_rec(
    FILE *file,           /*!< in: file where to print */
    const rec_t *rec,     /*!< in: physical record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec);
  ut_ad(offsets);
  ut_ad(rec_offs_validate(rec, NULL, offsets));

  if (!rec_offs_comp(offsets)) {
    rec_print_mbr_old(file, rec);
    return;
  }

  for (ulint i = 0; i < rec_offs_n_fields(offsets); i++) {
    const byte *data;
    ulint len;

    ut_ad(!rec_offs_nth_default(offsets, i));
    data = rec_get_nth_field(rec, offsets, i, &len);

    if (i == 0) {
      fprintf(file, " MBR:");
      for (; len > 0; len -= sizeof(double)) {
        double d = mach_double_read(data);

        if (len != sizeof(double)) {
          fprintf(file, "%.2lf,", d);
        } else {
          fprintf(file, "%.2lf", d);
        }

        data += sizeof(double);
      }
    } else {
      fprintf(file, " %lu:", (ulong)i);

      if (len != UNIV_SQL_NULL) {
        if (len <= 30) {
          ut_print_buf(file, data, len);
        } else {
          ut_print_buf(file, data, 30);

          fprintf(file, " (total %lu bytes)", (ulong)len);
        }
      } else {
        fputs(" SQL NULL", file);
      }
    }
    putc(';', file);
  }

  if (rec_get_info_bits(rec, true) & REC_INFO_DELETED_FLAG) {
    fprintf(file, " Deleted");
  }

  if (rec_get_info_bits(rec, true) & REC_INFO_MIN_REC_FLAG) {
    fprintf(file, " First rec");
  }

  rec_validate(rec, offsets);
}



#endif
