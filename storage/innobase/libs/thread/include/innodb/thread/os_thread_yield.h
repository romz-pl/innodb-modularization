#pragma once

#include <innodb/thread/os_thread_yield.h>
#include <thread>

/** Advises the OS to give up remainder of the thread's time slice. */
#define os_thread_yield()      \
  do {                         \
    std::this_thread::yield(); \
  } while (false)

