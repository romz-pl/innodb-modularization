#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_id_t.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_PFS_MUTEX
#ifndef UNIV_HOTBACKUP

/** String representation of the filename and line number where the
latch was created
@param[in]	id		Latch ID
@param[in]	created		Filename and line number where it was crated
@return the string representation */
std::string sync_mutex_to_string(latch_id_t id, const std::string &created);

#endif
#endif
#endif
