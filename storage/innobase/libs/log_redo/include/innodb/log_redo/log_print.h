#pragma once

#include <innodb/univ/univ.h>

#include <stdio.h>

struct log_t;

/** Prints information about important lsn values used in the redo log,
and some statistics about speed of writing and flushing of data.
@param[in]	log	redo log for which print information
@param[out]	file	file where to print */
void log_print(const log_t &log, FILE *file);
