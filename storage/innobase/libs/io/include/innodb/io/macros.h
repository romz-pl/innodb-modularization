#pragma once

#include <innodb/univ/univ.h>

/** Win NT does not allow more than 64 */
static const ulint OS_AIO_N_PENDING_IOS_PER_THREAD = 32;

#ifdef _WIN32

/* On Windows when using native AIO the number of AIO requests
that a thread can handle at a given time is limited to 32
i.e.: SRV_N_PENDING_IOS_PER_THREAD */
#define SRV_N_PENDING_IOS_PER_THREAD OS_AIO_N_PENDING_IOS_PER_THREAD

#endif /* _WIN32 */


#include "my_io.h"

/* Maximum path string length in bytes when referring to tables with in the
'./databasename/tablename.ibd' path format; we can allocate at least 2 buffers
of this size from the thread stack; that is why this should not be made much
bigger than 4000 bytes.  The maximum path length used by any storage engine
in the server must be at least this big. */
#define OS_FILE_MAX_PATH 4000
#if (FN_REFLEN_SE < OS_FILE_MAX_PATH)
#error "(FN_REFLEN_SE < OS_FILE_MAX_PATH)"
#endif
