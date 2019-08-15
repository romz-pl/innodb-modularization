#pragma once

#include <innodb/univ/univ.h>


#include <innodb/dict_mem/dict_index_t.h>

/** Request a lazy creation of dict_index_t::zip_pad::mutex.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	index	index whose zip_pad mutex is to be created */
inline void dict_index_zip_pad_mutex_create_lazy(dict_index_t *index) {
  index->zip_pad.mutex = NULL;
  index->zip_pad.mutex_created = os_once::NEVER_DONE;
}
