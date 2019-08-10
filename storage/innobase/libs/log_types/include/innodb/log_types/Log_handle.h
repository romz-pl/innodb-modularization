#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_lock_no_t.h>
#include <innodb/log_sn/lsn_t.h>

struct Log_handle {
  log_lock_no_t lock_no;

  lsn_t start_lsn;

  lsn_t end_lsn;
};
