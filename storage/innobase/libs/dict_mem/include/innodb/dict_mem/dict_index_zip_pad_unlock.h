#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Release the zip_pad_mutex of a given index.
@param[in,out]	index	index whose zip_pad_mutex is to be released */
inline void dict_index_zip_pad_unlock(dict_index_t *index) {
#ifndef UNIV_HOTBACKUP
  mutex_exit(index->zip_pad.mutex);
#endif /* !UNIV_HOTBACKUP */
}
