#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>
#include <innodb/error/dberr_t.h>
#include <innodb/log_types/checkpoint_no_t.h>

struct log_t;

dberr_t recv_log_recover_pre_8_0_4(log_t &log,
                                          checkpoint_no_t checkpoint_no,
                                          lsn_t checkpoint_lsn);
