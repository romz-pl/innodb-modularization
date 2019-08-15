#include <innodb/dict_mem/dict_mem_index_free.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_zip_pad_mutex_destroy.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>
#include <innodb/sync_mutex/mutex_destroy.h>
#include <innodb/memory/mem_heap_free.h>
#include <innodb/dict_mem/dict_index_remove_from_v_col_list.h>

#include "sql/dd/types/spatial_reference_system.h"

/** Frees an index memory object. */
void dict_mem_index_free(dict_index_t *index) /*!< in: index */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

#ifndef UNIV_HOTBACKUP
  dict_index_zip_pad_mutex_destroy(index);

  if (dict_index_is_spatial(index)) {
    rtr_info_active::iterator it;
    rtr_info_t *rtr_info;

    for (it = index->rtr_track->rtr_active->begin();
         it != index->rtr_track->rtr_active->end(); ++it) {
      rtr_info = *it;

      rtr_info->index = NULL;
    }

    mutex_destroy(&index->rtr_ssn.mutex);
    mutex_destroy(&index->rtr_track->rtr_active_mutex);
    UT_DELETE(index->rtr_track->rtr_active);
  }
  dict_index_remove_from_v_col_list(index);
#endif /* !UNIV_HOTBACKUP */

  index->rtr_srs.reset();

  mem_heap_free(index->heap);
}
