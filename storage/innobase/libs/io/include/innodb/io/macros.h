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
