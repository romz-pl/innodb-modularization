#pragma once

#include <innodb/univ/univ.h>

#ifdef PAGE_ZIP_COMPRESS_DBG

/** The logfile parameter */
#define LOGFILE logfile,

#else /* PAGE_ZIP_COMPRESS_DBG */

/** Missing logfile parameter */
#define LOGFILE

#endif /* PAGE_ZIP_COMPRESS_DBG */
