#pragma once

#include <innodb/univ/univ.h>
#include <string>

#ifndef UNIV_LIBRARY

/** Get the string where the file was created. Its format is "name:line"
@param[in]	ptr		Latch instance
@return created information or "" if can't be found */
std::string sync_file_created_get(const void *ptr);

#endif
