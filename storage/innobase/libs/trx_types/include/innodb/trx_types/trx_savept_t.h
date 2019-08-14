#pragma once

#include <innodb/univ/univ.h>

/** Transaction savepoint */
struct trx_savept_t {
  undo_no_t least_undo_no; /*!< least undo number to undo */
};
