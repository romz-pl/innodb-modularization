#pragma once

#include <innodb/univ/univ.h>

/** Number of spin iterations, for which log writer thread is waiting
for new data to write or flush without sleeping. */
extern ulong srv_log_writer_spin_delay;
