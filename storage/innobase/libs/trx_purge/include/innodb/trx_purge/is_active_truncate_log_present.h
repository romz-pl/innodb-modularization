#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Check if TRUNCATE_DDL_LOG file exist.
@param[in]  space_num  undo tablespace number
@return true if exist else false. */
bool is_active_truncate_log_present(space_id_t space_num);

}
