#include <innodb/record/rec_offs_any_null_extern.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/flag.h>

/** Determine if the offsets are for a record containing null BLOB pointers.
 @return first field containing a null BLOB pointer, or NULL if none found */
const byte *rec_offs_any_null_extern(
    const rec_t *rec,     /*!< in: record */
    const ulint *offsets) /*!< in: rec_get_offsets(rec) */
{
  ulint i;
  ut_ad(rec_offs_validate(rec, NULL, offsets));

  if (!rec_offs_any_extern(offsets)) {
    return (NULL);
  }

  for (i = 0; i < rec_offs_n_fields(offsets); i++) {
    if (rec_offs_nth_extern(offsets, i)) {
      ulint len;
      const byte *field = rec_get_nth_field(rec, offsets, i, &len);

      ut_a(len >= BTR_EXTERN_FIELD_REF_SIZE);
      if (!memcmp(field + len - BTR_EXTERN_FIELD_REF_SIZE, field_ref_zero,
                  BTR_EXTERN_FIELD_REF_SIZE)) {
        return (field);
      }
    }
  }

  return (NULL);
}
