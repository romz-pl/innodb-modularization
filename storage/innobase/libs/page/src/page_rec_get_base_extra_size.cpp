#include <innodb/page/page_rec_get_base_extra_size.h>

#include <innodb/page/page_rec_is_comp.h>
#include <innodb/page/header.h>

/** Returns the base extra size of a physical record.  This is the
 size of the fixed header, independent of the record size.
 @return REC_N_NEW_EXTRA_BYTES or REC_N_OLD_EXTRA_BYTES */
ulint page_rec_get_base_extra_size(const rec_t *rec) /*!< in: physical record */
{
#if REC_N_NEW_EXTRA_BYTES + 1 != REC_N_OLD_EXTRA_BYTES
#error "REC_N_NEW_EXTRA_BYTES + 1 != REC_N_OLD_EXTRA_BYTES"
#endif
  return (REC_N_NEW_EXTRA_BYTES + (ulint)!page_rec_is_comp(rec));
}
