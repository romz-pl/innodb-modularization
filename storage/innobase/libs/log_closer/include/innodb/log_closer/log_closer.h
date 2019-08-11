#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** The log closer thread co-routine.
@see @ref sect_redo_log_closer
@param[in,out]	log_ptr		pointer to redo log */
void log_closer(log_t *log_ptr);
