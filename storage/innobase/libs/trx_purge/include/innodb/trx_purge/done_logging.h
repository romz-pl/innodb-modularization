#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Mark completion of undo truncate action by writing magic number
to the log file and then removing it from the disk.
If we are going to remove it from disk then why write magic number?
This is to safeguard from unlink (file-system) anomalies that will
keep the link to the file even after unlink action is successful
and ref-count = 0.
@param[in]  space_num  number of the undo tablespace to truncate. */
void done_logging(space_id_t space_num);

}
