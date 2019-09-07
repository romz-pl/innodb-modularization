#pragma once

#include <innodb/univ/univ.h>

/** Create the requested number of Rollback Segments in a newly created undo
tablespace and add them to the Rsegs object.
@param[in]  space_id                  undo tablespace ID
@param[in]  target_rollback_segments  number of rollback segments per space
@return true if all necessary rollback segments and trx_rseg_t objects
were created. */
bool trx_rseg_init_rollback_segments(space_id_t space_id,
                                     ulong target_rollback_segments);
