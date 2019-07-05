#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_HOTBACKUP
// MAHI: changed from 130 to 1 assuming the apply-log is single threaded
#define SRV_MAX_N_IO_THREADS 1
#else /* UNIV_HOTBACKUP */
#define SRV_MAX_N_IO_THREADS 130
#endif /* UNIV_HOTBACKUP */
