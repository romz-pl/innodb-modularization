#pragma once

#include <innodb/univ/univ.h>
#include <innodb/thread/os_thread_get_curr_id.h>

/** Return the thread handle. The purpose of this function is to cast the
native handle to an integer type for consistency
@return the current thread ID cast to an uint64_t */
#define os_thread_handle() ((uint64_t)(os_thread_get_curr_id()))
