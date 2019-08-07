#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_redo/log_test.h>

#include "my_dbug.h"
#include "my_sys.h"

#define LOG_SYNC_POINT(a)                \
  do {                                   \
    DEBUG_SYNC_C(a);                     \
    DBUG_EXECUTE_IF(a, DBUG_SUICIDE();); \
    if (log_test != nullptr) {           \
      log_test->sync_point(a);           \
    }                                    \
  } while (0)
