#include <innodb/record/rec_convert_dtuple_to_rec_old.h>

#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dtuple_get_data_size.h>
#include <innodb/data_types/dtuple_get_info_bits.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtype_get_sql_null_size.h>
#include <innodb/record/data_write_sql_null.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_1_set_field_end_info.h>
#include <innodb/record/rec_2_set_field_end_info.h>
#include <innodb/record/rec_get_converted_extra_size.h>
#include <innodb/record/rec_set_1byte_offs_flag.h>
#include <innodb/record/rec_set_info_bits_old.h>
#include <innodb/record/rec_set_n_fields_old.h>

/** Builds an old-style physical record out of a data tuple and
 stores it beginning from the start of the given buffer.
 @return pointer to the origin of physical record */
rec_t *rec_convert_dtuple_to_rec_old(
    byte *buf,              /*!< in: start address of the physical record */
    const dtuple_t *dtuple, /*!< in: data tuple */
    ulint n_ext)            /*!< in: number of externally stored columns */
{
  const dfield_t *field;
  ulint n_fields;
  ulint data_size;
  rec_t *rec;
  ulint end_offset;
  ulint ored_offset;
  ulint len;
  ulint i;

  ut_ad(buf && dtuple);
  ut_ad(dtuple_validate(dtuple));
  ut_ad(dtuple_check_typed(dtuple));

  n_fields = dtuple_get_n_fields(dtuple);
  data_size = dtuple_get_data_size(dtuple, 0);

  ut_ad(n_fields > 0);

  /* Calculate the offset of the origin in the physical record */

  rec = buf + rec_get_converted_extra_size(data_size, n_fields, n_ext);
#ifdef UNIV_DEBUG
  /* Suppress Valgrind warnings of ut_ad()
  in mach_write_to_1(), mach_write_to_2() et al. */
  memset(buf, 0xff, rec - buf + data_size);
#endif /* UNIV_DEBUG */
  /* Store the number of fields */
  rec_set_n_fields_old(rec, n_fields);

  /* Set the info bits of the record */
  rec_set_info_bits_old(rec, dtuple_get_info_bits(dtuple) & REC_INFO_BITS_MASK);

  /* Store the data and the offsets */

  end_offset = 0;

  if (!n_ext && data_size <= REC_1BYTE_OFFS_LIMIT) {
    rec_set_1byte_offs_flag(rec, TRUE);

    for (i = 0; i < n_fields; i++) {
      field = dtuple_get_nth_field(dtuple, i);

      if (dfield_is_null(field)) {
        len = dtype_get_sql_null_size(dfield_get_type(field), 0);
        data_write_sql_null(rec + end_offset, len);

        end_offset += len;
        ored_offset = end_offset | REC_1BYTE_SQL_NULL_MASK;
      } else {
        /* If the data is not SQL null, store it */
        len = dfield_get_len(field);

        memcpy(rec + end_offset, dfield_get_data(field), len);

        end_offset += len;
        ored_offset = end_offset;
      }

      rec_1_set_field_end_info(rec, i, ored_offset);
    }
  } else {
    rec_set_1byte_offs_flag(rec, FALSE);

    for (i = 0; i < n_fields; i++) {
      field = dtuple_get_nth_field(dtuple, i);

      if (dfield_is_null(field)) {
        len = dtype_get_sql_null_size(dfield_get_type(field), 0);
        data_write_sql_null(rec + end_offset, len);

        end_offset += len;
        ored_offset = end_offset | REC_2BYTE_SQL_NULL_MASK;
      } else {
        /* If the data is not SQL null, store it */
        len = dfield_get_len(field);

        memcpy(rec + end_offset, dfield_get_data(field), len);

        end_offset += len;
        ored_offset = end_offset;

        if (dfield_is_ext(field)) {
          ored_offset |= REC_2BYTE_EXTERN_MASK;
        }
      }

      rec_2_set_field_end_info(rec, i, ored_offset);
    }
  }

  return (rec);
}

