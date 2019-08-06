#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Get the logging mode of a mini-transaction.
@return	logging mode: MTR_LOG_NONE, ... */
#define mtr_get_log_mode(m) (m)->get_log_mode()
