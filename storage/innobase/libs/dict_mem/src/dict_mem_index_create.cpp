#include <innodb/dict_mem/dict_mem_index_create.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_zip_pad_mutex_create_lazy.h>
#include <innodb/dict_mem/dict_mem_fill_index_struct.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/memory/mem_heap_create.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/sync_mutex/mutex_create.h>

/** Creates an index memory object.
 @return own: index object */
dict_index_t *dict_mem_index_create(
    const char *table_name, /*!< in: table name */
    const char *index_name, /*!< in: index name */
    ulint space,            /*!< in: space where the index tree is
                            placed, ignored if the index is of
                            the clustered type */
    ulint type,             /*!< in: DICT_UNIQUE,
                            DICT_CLUSTERED, ... ORed */
    ulint n_fields)         /*!< in: number of fields */
{
  dict_index_t *index;
  mem_heap_t *heap;

  ut_ad(table_name && index_name);

  heap = mem_heap_create(DICT_HEAP_SIZE);

  index = static_cast<dict_index_t *>(mem_heap_zalloc(heap, sizeof(*index)));

  dict_mem_fill_index_struct(index, heap, table_name, index_name, space, type,
                             n_fields);

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  dict_index_zip_pad_mutex_create_lazy(index);

  if (type & DICT_SPATIAL) {
    mutex_create(LATCH_ID_RTR_SSN_MUTEX, &index->rtr_ssn.mutex);
    index->rtr_track = static_cast<rtr_info_track_t *>(
        mem_heap_alloc(heap, sizeof(*index->rtr_track)));
    mutex_create(LATCH_ID_RTR_ACTIVE_MUTEX,
                 &index->rtr_track->rtr_active_mutex);
    index->rtr_track->rtr_active = UT_NEW_NOKEY(rtr_info_active());
  }
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */

  return (index);
}

