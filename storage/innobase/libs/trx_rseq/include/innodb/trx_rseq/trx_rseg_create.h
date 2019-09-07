#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

/** Create a rollback segment in the given tablespace. This could be either
the system tablespace, the temporary tablespace, or an undo tablespace.
@param[in]	space_id	tablespace to get the rollback segment
@param[in]	rseg_id		slot number of the rseg within this tablespace
@return page number of the rollback segment header page created */
page_no_t trx_rseg_create(space_id_t space_id, ulint rseg_id);
