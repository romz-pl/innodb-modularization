#include <innodb/record/rec_get_n_fields_instant.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Get the number of fields for one new style leaf page record.
This is only needed for table after instant ADD COLUMN.
@param[in]	rec		leaf page record
@param[in]	extra_bytes	extra bytes of this record
@param[in,out]	length		length of number of fields
@return	number of fields */
uint32_t rec_get_n_fields_instant(const rec_t *rec, const ulint extra_bytes,
                                  uint16_t *length) {
  uint16_t n_fields;
  const byte *ptr;

  ptr = rec - (extra_bytes + 1);

  if ((*ptr & REC_N_FIELDS_TWO_BYTES_FLAG) == 0) {
    *length = 1;
    return (*ptr);
  }

  *length = 2;
  n_fields = ((*ptr-- & REC_N_FIELDS_ONE_BYTE_MAX) << 8);
  n_fields |= *ptr;
  ut_ad(n_fields < REC_MAX_N_FIELDS);
  ut_ad(n_fields != 0);

  return (n_fields);
}
