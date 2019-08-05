#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/error/dberr_t.h>

struct trx_t;

/** Closes a single-table tablespace. The tablespace must be cached in the
memory cache. Free all pages used by the tablespace.
@param[in,out]	trx		Transaction covering the close
@param[in]	space_id	Tablespace ID
@return DB_SUCCESS or error */
dberr_t fil_close_tablespace(trx_t *trx, space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
