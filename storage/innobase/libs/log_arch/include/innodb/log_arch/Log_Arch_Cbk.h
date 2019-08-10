#pragma once

#include <innodb/univ/univ.h>


/** File Node Iterator callback
@param[in]	file_name	NULL terminated file name
@param[in]	file_size	size of file in bytes
@param[in]	read_offset	offset to start reading from
@param[in]	ctx		context passed by caller
@return error code */
using Log_Arch_Cbk = int(char *file_name, ib_uint64_t file_size,
                         ib_uint64_t read_offset, void *ctx);
