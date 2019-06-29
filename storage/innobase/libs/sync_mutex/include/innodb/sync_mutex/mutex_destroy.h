#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/**
Removes a mutex instance from the mutex list. The mutex is checked to
be in the reset state.
@param[in,out]	 mutex		mutex instance to destroy */
template <typename Mutex>
void mutex_destroy(Mutex *mutex) {
  mutex->destroy();
}

#endif
#endif
