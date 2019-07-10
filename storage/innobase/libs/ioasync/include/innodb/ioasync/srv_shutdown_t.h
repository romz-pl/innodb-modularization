#pragma once

#include <innodb/univ/univ.h>

/** Shutdown state */
enum srv_shutdown_t {
  SRV_SHUTDOWN_NONE = 0,    /*!< Database running normally */
  SRV_SHUTDOWN_CLEANUP,     /*!< Cleaning up in
                            logs_empty_and_mark_files_at_shutdown() */
  SRV_SHUTDOWN_FLUSH_PHASE, /*!< At this phase the master and the
                           purge threads must have completed their
                           work. Once we enter this phase the
                           page_cleaner can clean up the buffer
                           pool and exit */
  SRV_SHUTDOWN_LAST_PHASE,  /*!< Last phase after ensuring that
                            the buffer pool can be freed: flush
                            all file spaces and close all files */
  SRV_SHUTDOWN_EXIT_THREADS /*!< Exit all threads */
};
