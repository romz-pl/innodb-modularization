#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Returns nonzero if the field is stored off-page.
 @retval 0 if the field is stored in-page
 @retval REC_2BYTE_EXTERN_MASK if the field is stored externally */
ulint rec_2_is_field_extern(const rec_t *rec, /*!< in: record */
                            ulint n)          /*!< in: field index */
    MY_ATTRIBUTE((warn_unused_result));
