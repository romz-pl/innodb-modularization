#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** The log writer thread co-routine.
@see @ref sect_redo_log_writer
@param[in,out]	log_ptr		pointer to redo log */
void log_writer(log_t *log_ptr);
