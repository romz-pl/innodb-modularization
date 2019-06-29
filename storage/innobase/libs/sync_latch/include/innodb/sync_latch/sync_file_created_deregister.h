#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY

/** Deregister a latch, called when it is destroyed
@param[in]	ptr		Latch to be destroyed */
void sync_file_created_deregister(const void *ptr);

#endif
