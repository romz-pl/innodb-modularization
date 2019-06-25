#pragma once

#include <innodb/univ/univ.h>

#ifdef _WIN32

/** Initialise highest available time resolution API on Windows
 @return false if all OK else true */
bool ut_win_init_time();

#endif /* _WIN32 */
