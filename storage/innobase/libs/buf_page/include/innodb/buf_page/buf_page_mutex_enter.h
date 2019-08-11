#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_mutex/mutex_enter.h>

/** Acquire the block->mutex. */
#define buf_page_mutex_enter(b) \
  do {                          \
    mutex_enter(&(b)->mutex);   \
  } while (0)


#endif
