#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function tells if record is delete marked.
 @return nonzero if delete marked */
ulint rec_get_deleted_flag(const rec_t *rec, /*!< in: physical record */
                           ulint comp) /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));
