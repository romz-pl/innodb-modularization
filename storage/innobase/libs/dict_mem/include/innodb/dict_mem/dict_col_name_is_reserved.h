#pragma once

#include <innodb/univ/univ.h>

/** If the given column name is reserved for InnoDB system columns, return
 TRUE.
 @return true if name is reserved */
ibool dict_col_name_is_reserved(const char *name) /*!< in: column name */
    MY_ATTRIBUTE((warn_unused_result));
