#include <innodb/data_types/dfield_print_also_hex.h>

#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dtype_get_prtype.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dtype_get_mtype.h>
#include <innodb/machine/data.h>
#include <innodb/formatting/formatting.h>
#include <innodb/data_types/flags.h>

#include <cstdio>

#ifndef UNIV_HOTBACKUP

/** Pretty prints a dfield value according to its data type. Also the hex string
 is printed if a string contains non-printable characters. */
void dfield_print_also_hex(const dfield_t *dfield) /*!< in: dfield */
{
  const byte *data;
  ulint len;
  ulint prtype;
  ulint i;
  ibool print_also_hex;

  len = dfield_get_len(dfield);
  data = static_cast<const byte *>(dfield_get_data(dfield));

  if (dfield_is_null(dfield)) {
    fputs("NULL", stderr);

    return;
  }

  prtype = dtype_get_prtype(dfield_get_type(dfield));

  switch (dtype_get_mtype(dfield_get_type(dfield))) {
    ib_id_t id;
    case DATA_INT:
      switch (len) {
        ulint val;
        case 1:
          val = mach_read_from_1(data);

          if (!(prtype & DATA_UNSIGNED)) {
            val &= ~0x80;
            fprintf(stderr, "%ld", (long)val);
          } else {
            fprintf(stderr, "%lu", (ulong)val);
          }
          break;

        case 2:
          val = mach_read_from_2(data);

          if (!(prtype & DATA_UNSIGNED)) {
            val &= ~0x8000;
            fprintf(stderr, "%ld", (long)val);
          } else {
            fprintf(stderr, "%lu", (ulong)val);
          }
          break;

        case 3:
          val = mach_read_from_3(data);

          if (!(prtype & DATA_UNSIGNED)) {
            val &= ~0x800000;
            fprintf(stderr, "%ld", (long)val);
          } else {
            fprintf(stderr, "%lu", (ulong)val);
          }
          break;

        case 4:
          val = mach_read_from_4(data);

          if (!(prtype & DATA_UNSIGNED)) {
            val &= ~0x80000000;
            fprintf(stderr, "%ld", (long)val);
          } else {
            fprintf(stderr, "%lu", (ulong)val);
          }
          break;

        case 6:
          id = mach_read_from_6(data);
          fprintf(stderr, IB_ID_FMT, id);
          break;

        case 7:
          id = mach_read_from_7(data);
          fprintf(stderr, IB_ID_FMT, id);
          break;
        case 8:
          id = mach_read_from_8(data);
          fprintf(stderr, IB_ID_FMT, id);
          break;
        default:
          goto print_hex;
      }
      break;

    case DATA_SYS:
      switch (prtype & DATA_SYS_PRTYPE_MASK) {
        case DATA_TRX_ID:
          id = mach_read_from_6(data);

          fprintf(stderr, "trx_id " TRX_ID_FMT, id);
          break;

        case DATA_ROLL_PTR:
          id = mach_read_from_7(data);

          fprintf(stderr, "roll_ptr " TRX_ID_FMT, id);
          break;

        case DATA_ROW_ID:
          id = mach_read_from_6(data);

          fprintf(stderr, "row_id " TRX_ID_FMT, id);
          break;

        default:
          goto print_hex;
      }
      break;

    case DATA_CHAR:
    case DATA_VARCHAR:
      print_also_hex = FALSE;

      for (i = 0; i < len; i++) {
        int c = *data++;

        if (!isprint(c)) {
          print_also_hex = TRUE;

          fprintf(stderr, "\\x%02x", (unsigned char)c);
        } else {
          putc(c, stderr);
        }
      }

      if (dfield_is_ext(dfield)) {
        fputs("(external)", stderr);
      }

      if (!print_also_hex) {
        break;
      }

      data = static_cast<byte *>(dfield_get_data(dfield));
      /* fall through */

    case DATA_BINARY:
    default:
    print_hex:
      fputs(" Hex: ", stderr);

      for (i = 0; i < len; i++) {
        fprintf(stderr, "%02lx", static_cast<ulong>(*data++));
      }

      if (dfield_is_ext(dfield)) {
        fputs("(external)", stderr);
      }
  }
}




#endif
