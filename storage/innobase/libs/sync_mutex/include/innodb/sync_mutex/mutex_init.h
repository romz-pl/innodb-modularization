#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

#include <innodb/sync_latch/latch_id_t.h>

/**
Creates, or rather, initializes a mutex object in a specified memory
location (which must be appropriately aligned). The mutex is initialized
in the reset state. Explicit freeing of the mutex with mutex_free is
necessary only if the memory block containing it is freed.
Add the mutex instance to the global mutex list.
@param[in,out]	mutex		mutex to initialise
@param[in]	id		The mutex ID (Latch ID)
@param[in]	file_name	Filename from where it was called
@param[in]	line		Line number in filename from where called */
template <typename Mutex>
void mutex_init(Mutex *mutex, latch_id_t id, const char *file_name,
                uint32_t line) {
  new (mutex) Mutex();

  mutex->init(id, file_name, line);
}

#endif
#endif

