#include <innodb/page/page_zip_fields_free.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_zip_pad_mutex_destroy.h>
#include <innodb/memory/mem_heap_free.h>

void dict_mem_table_free(dict_table_t *table);

/** Deallocate the index information initialized by page_zip_fields_decode(). */
void page_zip_fields_free(
    dict_index_t *index) /*!< in: dummy index to be freed */
{
  if (index) {
    dict_table_t *table = index->table;
#ifndef UNIV_HOTBACKUP
    dict_index_zip_pad_mutex_destroy(index);
#endif /* !UNIV_HOTBACKUP */
    mem_heap_free(index->heap);

    dict_mem_table_free(table);
  }
}
