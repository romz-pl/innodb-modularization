#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/Space_Ids.h>

namespace undo {

/** list of undo tablespaces that need header pages and rollback
segments written to them at startup.  This can be because they are
newly initialized, were being truncated and the system crashed, or
they were an old format at startup and were replaced when they were
opened. Old format undo tablespaces do not have space_ids between
dict_sys_t_s_min_undo_space_id and dict_sys_t_s_max_undo_space_id
and they do not contain an RSEG_ARRAY page. */
extern Space_Ids s_under_construction;

}
