#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Updates sn limit values up to which user threads may consider the
reserved space as available both in the log buffer and in the log files.
Both limits - for the start and for the end of reservation, are updated.
Limit for the end is the only one, which truly guarantees that there is
space for the whole reservation. Limit for the start is used to check
free space when being outside mtr (without latches), in which case it
is unknown how much we will need to reserve and write, so current sn
is then compared to the limit. This is called whenever these limits
may change - when write_lsn or last_checkpoint_lsn are advanced,
when the log buffer is resized or margins are changed (e.g. because
of changed concurrency limit).
@param[in,out]	log	redo log */
void log_update_limits(log_t &log);
