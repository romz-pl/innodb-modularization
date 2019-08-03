#include <innodb/record/rec_copy.h>

#include <innodb/assert/assert.h>
#include <innodb/memory/ut_memcpy.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_extra_size.h>
#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_validate.h>

/** Copy a physical record to a buffer.
@param[in]	buf	buffer
@param[in]	rec	physical record
@param[in]	offsets	array returned by rec_get_offsets()
@return pointer to the origin of the copy */
rec_t *rec_copy(void *buf, const rec_t *rec, const ulint *offsets) {
  ulint extra_len;
  ulint data_len;

  ut_ad(rec != NULL);
  ut_ad(buf != NULL);
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  ut_ad(rec_validate(rec, offsets));

  extra_len = rec_offs_extra_size(offsets);
  data_len = rec_offs_data_size(offsets);

  ut_memcpy(buf, rec - extra_len, extra_len + data_len);

  return ((byte *)buf + extra_len);
}
