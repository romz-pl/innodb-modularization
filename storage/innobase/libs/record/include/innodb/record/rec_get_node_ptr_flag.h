#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function tells if a new-style record is a node pointer.
 @return true if node pointer */
bool rec_get_node_ptr_flag(const rec_t *rec) /*!< in: physical record */
    MY_ATTRIBUTE((warn_unused_result));
