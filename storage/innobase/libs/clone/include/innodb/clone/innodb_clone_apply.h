#pragma once

#include <innodb/univ/univ.h>

#include "sql/handler.h"

/** Apply data to destination database in chunks via callback
@param[in]	hton	handlerton for SE
@param[in]	thd	server thread handle
@param[in]	loc	locator
@param[in]	loc_len	locator length in bytes
@param[in]	task_id	task identifier
@param[in]	in_err	inform any error occurred
@param[in]	cbk	callback interface for receiving data
@return error code */
int innodb_clone_apply(handlerton *hton, THD *thd, const byte *loc,
                       uint loc_len, uint task_id, int in_err,
                       Ha_clone_cbk *cbk);
