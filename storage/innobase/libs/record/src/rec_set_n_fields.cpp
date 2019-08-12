#include <innodb/record/rec_set_n_fields.h>

#include <innodb/record/flag.h>

/** Set the number of fields for one new style leaf page record.
This is only needed for table after instant ADD COLUMN.
@param[in,out]	rec		leaf page record
@param[in]	n_fields	number of fields in the record
@return	the length of the n_fields occupies */
uint8_t rec_set_n_fields(rec_t *rec, ulint n_fields) {
  byte *ptr = rec - (REC_N_NEW_EXTRA_BYTES + 1);

  ut_ad(n_fields < REC_MAX_N_FIELDS);

  if (n_fields <= REC_N_FIELDS_ONE_BYTE_MAX) {
    *ptr = static_cast<byte>(n_fields);
    return (1);
  }

  --ptr;
  *ptr++ = static_cast<byte>(n_fields & 0xFF);
  *ptr = static_cast<byte>(n_fields >> 8);
  ut_ad((*ptr & 0x80) == 0);
  *ptr |= REC_N_FIELDS_TWO_BYTES_FLAG;

  return (2);
}
