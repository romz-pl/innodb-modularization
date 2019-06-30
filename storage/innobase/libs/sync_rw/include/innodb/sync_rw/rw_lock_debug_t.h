#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG
/** The structure for storing debug info of an rw-lock.  All access to this
structure must be protected by rw_lock_debug_mutex_enter(). */
struct rw_lock_debug_t {
  os_thread_id_t thread_id; /*!< The thread id of the thread which
                         locked the rw-lock */
  ulint pass;               /*!< Pass value given in the lock operation */
  ulint lock_type;          /*!< Type of the lock: RW_LOCK_X,
                            RW_LOCK_S, RW_LOCK_X_WAIT */
  const char *file_name;    /*!< File name where the lock was obtained */
  ulint line;               /*!< Line where the rw-lock was locked */
  UT_LIST_NODE_T(rw_lock_debug_t) list;
  /*!< Debug structs are linked in a two-way
  list */
};
#endif /* UNIV_DEBUG */
