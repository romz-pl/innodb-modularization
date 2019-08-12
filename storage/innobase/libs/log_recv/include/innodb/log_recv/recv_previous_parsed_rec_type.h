#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>

/** The type of the previous parsed redo log record */
extern mlog_id_t recv_previous_parsed_rec_type;
