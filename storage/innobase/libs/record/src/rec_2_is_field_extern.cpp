#include <innodb/record/rec_2_is_field_extern.h>

#include <innodb/record/rec_2_get_field_end_info.h>
#include <innodb/record/flag.h>

/** Returns nonzero if the field is stored off-page.
 @retval 0 if the field is stored in-page
 @retval REC_2BYTE_EXTERN_MASK if the field is stored externally */
ulint rec_2_is_field_extern(const rec_t *rec, /*!< in: record */
                            ulint n)          /*!< in: field index */
{
  return (rec_2_get_field_end_info(rec, n) & REC_2BYTE_EXTERN_MASK);
}
