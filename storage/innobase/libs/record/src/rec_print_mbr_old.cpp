#include <innodb/record/rec_print_mbr_old.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/formatting/formatting.h>
#include <innodb/machine/data.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_deleted_flag.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_nth_field_old.h>
#include <innodb/record/rec_get_nth_field_size.h>
#include <innodb/record/rec_validate_old.h>

/** Prints an old-style spatial index record. */
void rec_print_mbr_old(FILE *file,       /*!< in: file where to print */
                              const rec_t *rec) /*!< in: physical record */
{
  const byte *data;
  ulint len;
  ulint n;
  ulint i;

  ut_ad(rec);

  n = rec_get_n_fields_old_raw(rec);

  fprintf(file,
          "PHYSICAL RECORD: n_fields %lu;"
          " %u-byte offsets; info bits %lu\n",
          (ulong)n, rec_get_1byte_offs_flag(rec) ? 1 : 2,
          (ulong)rec_get_info_bits(rec, FALSE));

  for (i = 0; i < n; i++) {
    data = rec_get_nth_field_old(rec, i, &len);

    fprintf(file, " %lu:", (ulong)i);

    if (len != UNIV_SQL_NULL) {
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
        if (len <= 30) {
          ut_print_buf(file, data, len);
        } else {
          ut_print_buf(file, data, 30);

          fprintf(file, " (total %lu bytes)", (ulong)len);
        }
      }
    } else {
      fprintf(file, " SQL NULL, size " ULINTPF " ",
              rec_get_nth_field_size(rec, i));
    }

    putc(';', file);
    putc('\n', file);
  }

  if (rec_get_deleted_flag(rec, false)) {
    fprintf(file, " Deleted");
  }

  if (rec_get_info_bits(rec, true) & REC_INFO_MIN_REC_FLAG) {
    fprintf(file, " First rec");
  }

  rec_validate_old(rec);
}



#endif
