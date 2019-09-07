#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

namespace undo {

struct Tablespace;

/** Create the truncate log file. Needed to track the state of truncate during
a crash. An auxiliary redo log file undo_<space_id>_trunc.log will be created
while the truncate of the UNDO is in progress. This file is required during
recovery to complete the truncate.
@param[in]  undo_space  undo tablespace to truncate.
@return DB_SUCCESS or error code.*/
dberr_t start_logging(Tablespace *undo_space);

}
