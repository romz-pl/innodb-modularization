#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/memory/mem_heap_strdup.h>

/** This function poplulates a dict_index_t index memory structure with
 supplied information. */
UNIV_INLINE
void dict_mem_fill_index_struct(
    dict_index_t *index,    /*!< out: index to be filled */
    mem_heap_t *heap,       /*!< in: memory heap */
    const char *table_name, /*!< in: table name */
    const char *index_name, /*!< in: index name */
    ulint space,            /*!< in: space where the index tree is
                            placed, ignored if the index is of
                            the clustered type */
    ulint type,             /*!< in: DICT_UNIQUE,
                            DICT_CLUSTERED, ... ORed */
    ulint n_fields)         /*!< in: number of fields */
{
  if (heap) {
    index->heap = heap;
    index->name = mem_heap_strdup(heap, index_name);
    index->fields = (dict_field_t *)mem_heap_alloc(
        heap, 1 + n_fields * sizeof(dict_field_t));
  } else {
    index->name = index_name;
    index->heap = NULL;
    index->fields = NULL;
  }

  /* Assign a ulint to a 4-bit-mapped field.
  Only the low-order 4 bits are assigned. */
  index->type = type;
#ifndef UNIV_HOTBACKUP
  index->space = (unsigned int)space;
  index->page = FIL_NULL;
  index->merge_threshold = DICT_INDEX_MERGE_THRESHOLD_DEFAULT;
#endif /* !UNIV_HOTBACKUP */
  index->table_name = table_name;
  index->n_fields = (unsigned int)n_fields;
  /* The '1 +' above prevents allocation
  of an empty mem block */
  index->allow_duplicates = false;
  index->nulls_equal = false;
  index->disable_ahi = false;
  index->last_ins_cur = nullptr;
  index->last_sel_cur = nullptr;
#ifndef UNIV_HOTBACKUP
  new (&index->rec_cache) rec_cache_t();

#endif /* UNIV_HOTBACKUP */
#ifdef UNIV_DEBUG
  index->magic_n = DICT_INDEX_MAGIC_N;
#endif /* UNIV_DEBUG */
}


