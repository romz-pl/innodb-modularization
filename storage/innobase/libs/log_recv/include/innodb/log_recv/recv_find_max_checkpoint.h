#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct log_t;

MY_ATTRIBUTE((warn_unused_result))
dberr_t recv_find_max_checkpoint(log_t &log, ulint *max_field);
