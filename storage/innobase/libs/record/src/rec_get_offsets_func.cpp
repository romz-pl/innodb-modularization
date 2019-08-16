#include <innodb/record/rec_get_offsets_func.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree_nonleaf.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/record/rec_get_n_fields_old.h>
#include <innodb/record/rec_offs_get_n_alloc.h>
#include <innodb/record/rec_offs_set_n_fields.h>
#include <innodb/record/rec_init_offsets.h>
#include <innodb/record/flag.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/record/rec_offs_set_n_alloc.h>
#include <innodb/memory/mem_heap_create_at.h>

/** The following function determines the offsets to each field
 in the record.	It can reuse a previously returned array.
 Note that after instant ADD COLUMN, if this is a record
 from clustered index, fields in the record may be less than
 the fields defined in the clustered index. So the offsets
 size is allocated according to the clustered index fields.
 @return the new offsets */
ulint *rec_get_offsets_func(
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets,            /*!< in/out: array consisting of
                               offsets[0] allocated elements,
                               or an array from rec_get_offsets(),
                               or NULL */
    ulint n_fields,            /*!< in: maximum number of
                              initialized fields
                               (ULINT_UNDEFINED if all fields) */
#ifdef UNIV_DEBUG
    const char *file,  /*!< in: file name where called */
    ulint line,        /*!< in: line number where called */
#endif                 /* UNIV_DEBUG */
    mem_heap_t **heap) /*!< in/out: memory heap */
{
  ulint n;
  ulint size;

  ut_ad(rec);
  ut_ad(index);
  ut_ad(heap);

  if (dict_table_is_comp(index->table)) {
    switch (UNIV_EXPECT(rec_get_status(rec), REC_STATUS_ORDINARY)) {
      case REC_STATUS_ORDINARY:
        n = dict_index_get_n_fields(index);
        break;
      case REC_STATUS_NODE_PTR:
        /* Node pointer records consist of the
        uniquely identifying fields of the record
        followed by a child page number field. */
        n = dict_index_get_n_unique_in_tree_nonleaf(index) + 1;
        break;
      case REC_STATUS_INFIMUM:
      case REC_STATUS_SUPREMUM:
        /* infimum or supremum record */
        n = 1;
        break;
      default:
        ut_error;
    }
  } else {
    n = rec_get_n_fields_old(rec, index);
  }

  if (UNIV_UNLIKELY(n_fields < n)) {
    n = n_fields;
  }

  /* The offsets header consists of the allocation size at
  offsets[0] and the REC_OFFS_HEADER_SIZE bytes. */
  size = n + (1 + REC_OFFS_HEADER_SIZE);

  if (UNIV_UNLIKELY(!offsets) ||
      UNIV_UNLIKELY(rec_offs_get_n_alloc(offsets) < size)) {
    if (UNIV_UNLIKELY(!*heap)) {
      *heap = mem_heap_create_at(size * sizeof(ulint), file, line);
    }
    offsets = static_cast<ulint *>(mem_heap_alloc(*heap, size * sizeof(ulint)));

    rec_offs_set_n_alloc(offsets, size);
  }

  rec_offs_set_n_fields(offsets, n);
  rec_init_offsets(rec, index, offsets);
  return (offsets);
}
