#include <innodb/log_redo/log_get_max_modified_age_async.h>

#include <innodb/log_types/log_sys.h>

#ifndef UNIV_HOTBACKUP

lsn_t log_get_max_modified_age_async() {
  return (log_sys->max_modified_age_async);
}

#endif
