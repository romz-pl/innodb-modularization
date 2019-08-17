#pragma once

#include <innodb/univ/univ.h>

#ifdef PAGE_ZIP_COMPRESS_DBG

/** Declaration of the logfile parameter */
#define FILE_LOGFILE FILE *logfile,

#else /* PAGE_ZIP_COMPRESS_DBG */

/** Empty declaration of the logfile parameter */
#define FILE_LOGFILE

#endif /* PAGE_ZIP_COMPRESS_DBG */
