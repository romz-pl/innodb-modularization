#pragma once

#include <innodb/univ/univ.h>

#include "sql/handler.h"

/** Begin copy from source database
@param[in]	hton	handlerton for SE
@param[in]	thd	server thread handle
@param[in,out]	loc	locator
@param[in,out]	loc_len	locator length
@param[out]	task_id	task identifier
@param[in]	type	clone type
@param[in]	mode	mode for starting clone
@return error code */
int innodb_clone_begin(handlerton *hton, THD *thd, const byte *&loc,
                       uint &loc_len, uint &task_id, Ha_clone_type type,
                       Ha_clone_mode mode);
