#include <innodb/log_write/compute_write_event_slot.h>

#include <innodb/log_types/log_t.h>


namespace Log_files_write_impl {

size_t compute_write_event_slot(const log_t &log, lsn_t lsn) {
  return ((lsn / OS_FILE_LOG_BLOCK_SIZE) & (log.write_events_size - 1));
}

}
