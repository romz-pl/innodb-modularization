#pragma once

#include <innodb/univ/univ.h>
#include <thread>
#include <chrono>

/** The thread sleeps at least the time given in microseconds.
@param[in]	usecs		time in microseconds */
#define os_thread_sleep(usecs)                                     \
  do {                                                             \
    std::this_thread::sleep_for(std::chrono::microseconds(usecs)); \
  } while (false)
