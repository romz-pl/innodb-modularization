#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** The log flusher thread co-routine.
@see @ref sect_redo_log_flusher
@param[in,out]	log_ptr		pointer to redo log */
void log_flusher(log_t *log_ptr);
