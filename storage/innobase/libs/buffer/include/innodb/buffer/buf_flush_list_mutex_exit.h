#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_mutex/mutex_exit.h>

/** Release the flush list mutex. */
#define buf_flush_list_mutex_exit(b)    \
  do {                                  \
    mutex_exit(&(b)->flush_list_mutex); \
  } while (0)


#endif
