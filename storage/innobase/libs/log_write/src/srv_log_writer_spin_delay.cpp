#include <innodb/log_write/srv_log_writer_spin_delay.h>

#include <innodb/log_types/flags.h>


/** Number of spin iterations, for which log writer thread is waiting
for new data to write or flush without sleeping. */
ulong srv_log_writer_spin_delay = INNODB_LOG_WRITER_SPIN_DELAY_DEFAULT;
