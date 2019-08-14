#pragma once

#include <innodb/univ/univ.h>


enum select_mode {
  SELECT_ORDINARY,    /* default behaviour */
  SELECT_SKIP_LOCKED, /* skip the row if row is locked */
  SELECT_NOWAIT       /* return immediately if row is locked */
};
