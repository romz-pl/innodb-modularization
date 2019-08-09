#pragma once

#include <innodb/univ/univ.h>

/** Callback for retrieving archived page IDs
@param[in]	ctx		context passed by caller
@param[in]	buff		buffer with page IDs
@param[in]	num_pages	number of page IDs in buffer
@return error code */
using Page_Arch_Cbk = int(void *ctx, byte *buff, uint num_pages);
