#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_mutex/ib_mutex_t.h>

/** Mutex protecting page_zip_stat_per_index */
extern ib_mutex_t page_zip_stat_per_index_mutex;

#endif
