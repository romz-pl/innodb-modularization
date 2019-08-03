#include <innodb/record/rec_copy_prefix_to_buf_old.h>

#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_set_n_fields_old.h>
#include <innodb/record/flag.h>
#include <innodb/memory/ut_memcpy.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>

/** Copies the first n fields of an old-style physical record
 to a new physical record in a buffer.
 @return own: copied record */
rec_t *rec_copy_prefix_to_buf_old(
    const rec_t *rec, /*!< in: physical record */
    ulint n_fields,   /*!< in: number of fields to copy */
    ulint area_end,   /*!< in: end of the prefix data */
    byte **buf,       /*!< in/out: memory buffer for
                      the copied prefix, or NULL */
    size_t *buf_size) /*!< in/out: buffer size */
{
  rec_t *copy_rec;
  ulint area_start;
  ulint prefix_len;

  if (rec_get_1byte_offs_flag(rec)) {
    area_start = REC_N_OLD_EXTRA_BYTES + n_fields;
  } else {
    area_start = REC_N_OLD_EXTRA_BYTES + 2 * n_fields;
  }

  prefix_len = area_start + area_end;

  if ((*buf == NULL) || (*buf_size < prefix_len)) {
    ut_free(*buf);
    *buf_size = prefix_len;
    *buf = static_cast<byte *>(ut_malloc_nokey(prefix_len));
  }

  ut_memcpy(*buf, rec - area_start, prefix_len);

  copy_rec = *buf + area_start;

  rec_set_n_fields_old(copy_rec, n_fields);

  return (copy_rec);
}
