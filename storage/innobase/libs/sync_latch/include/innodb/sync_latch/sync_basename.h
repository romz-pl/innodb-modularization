#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_id_t.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_PFS_MUTEX
#ifndef UNIV_HOTBACKUP

/** Print the filename "basename"
@return the basename */
const char *sync_basename(const char *filename);

#endif
#endif
#endif
