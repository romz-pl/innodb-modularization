#include <innodb/trx_purge/srv_purge_rseg_truncate_frequency.h>

/** Rate at which UNDO records should be purged. */
ulong srv_purge_rseg_truncate_frequency = 128;
