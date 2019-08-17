#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_lru/buf_LRU_stat_cur.h>

/** Increments the I/O counter in buf_LRU_stat_cur. */
#define buf_LRU_stat_inc_io() buf_LRU_stat_cur.io++
