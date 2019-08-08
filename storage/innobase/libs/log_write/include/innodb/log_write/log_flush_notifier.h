#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** The log flush notifier thread co-routine.
@see @ref sect_redo_log_flush_notifier
@param[in,out]	log_ptr		pointer to redo log */
void log_flush_notifier(log_t *log_ptr);
