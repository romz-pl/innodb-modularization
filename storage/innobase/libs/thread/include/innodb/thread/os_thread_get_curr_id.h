#pragma once

#include <innodb/thread/os_thread_id_t.h>

/** Returns the thread identifier of current thread. Currently the thread
identifier in Unix is the thread handle itself.
@return current thread native handle */
os_thread_id_t os_thread_get_curr_id();
