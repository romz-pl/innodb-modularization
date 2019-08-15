#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/sync_mutex/mutex_free.h>

/** Destroy the zip_pad_mutex of the given index.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	index	index whose stats latch to destroy */
inline void dict_index_zip_pad_mutex_destroy(dict_index_t *index) {
  if (index->zip_pad.mutex_created == os_once::DONE &&
      index->zip_pad.mutex != NULL) {
    mutex_free(index->zip_pad.mutex);
    UT_DELETE(index->zip_pad.mutex);
  }
}
