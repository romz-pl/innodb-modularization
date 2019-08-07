#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

/** Redo log system (singleton). */
extern log_t *log_sys;
