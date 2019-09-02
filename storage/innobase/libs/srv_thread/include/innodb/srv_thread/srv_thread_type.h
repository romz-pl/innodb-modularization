#pragma once

#include <innodb/univ/univ.h>

/** Types of threads existing in the system. */
enum srv_thread_type {
  SRV_NONE,   /*!< None */
  SRV_WORKER, /*!< threads serving parallelized
              queries and queries released from
              lock wait */
  SRV_PURGE,  /*!< Purge coordinator thread */
  SRV_MASTER  /*!< the master thread, (whose type
              number must be biggest) */
};
