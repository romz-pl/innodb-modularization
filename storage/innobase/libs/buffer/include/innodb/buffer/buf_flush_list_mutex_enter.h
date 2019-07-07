#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_mutex/mutex_enter.h>

/** Acquire the flush list mutex. */
#define buf_flush_list_mutex_enter(b)    \
  do {                                   \
    mutex_enter(&(b)->flush_list_mutex); \
  } while (0)

#endif
