#pragma once

#include <innodb/univ/univ.h>

#include "sql/handler.h"

/** End copy from source database
@param[in]	hton	handlerton for SE
@param[in]	thd	server thread handle
@param[in]	loc	locator
@param[in]	loc_len	locator length in bytes
@param[in]	task_id	task identifier
@param[in]	in_err	error code when ending after error
@return error code */
int innodb_clone_end(handlerton *hton, THD *thd, const byte *loc, uint loc_len,
                     uint task_id, int in_err);
