#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/sn_t.h>

struct log_t;

/** Waits until there is free space in the log buffer. The free space has to be
available for range of sn values ending at the provided sn.
@see @ref sect_redo_log_waiting_for_writer
@param[in]     log     redo log
@param[in]     end_sn  end of the range of sn values */
void log_wait_for_space_in_log_buf(log_t &log, sn_t end_sn);
