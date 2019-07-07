#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

/** Release the block->mutex. */
#define buf_page_mutex_exit(b) \
  do {                         \
    (b)->mutex.exit();         \
  } while (0)


#endif
