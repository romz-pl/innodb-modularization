#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY

/** Register a latch, called when it is created
@param[in]	ptr		Latch instance that was created
@param[in]	filename	Filename where it was created
@param[in]	line		Line number in filename */
void sync_file_created_register(const void *ptr, const char *filename,
                                uint16_t line);


#endif
