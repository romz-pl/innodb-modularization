#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/SysMutex.h>
#include <innodb/dict_mem/os_once.h>


/** Data structure to hold information about about how much space in
an uncompressed page should be left as padding to avoid compression
failures. This estimate is based on a self-adapting heuristic. */
struct zip_pad_info_t {
  SysMutex *mutex; /*!< mutex protecting the info */
  ulint pad;       /*!< number of bytes used as pad */
  ulint success;   /*!< successful compression ops during
                   current round */
  ulint failure;   /*!< failed compression ops during
                   current round */
  ulint n_rounds;  /*!< number of currently successful
                  rounds */
#ifndef UNIV_HOTBACKUP
  volatile os_once::state_t mutex_created;
  /*!< Creation state of mutex member */
#endif /* !UNIV_HOTBACKUP */
};
