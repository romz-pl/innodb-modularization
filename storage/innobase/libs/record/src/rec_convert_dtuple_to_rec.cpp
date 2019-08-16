#include <innodb/record/rec_convert_dtuple_to_rec.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_validate.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/record/rec_convert_dtuple_to_rec_new.h>
#include <innodb/record/rec_convert_dtuple_to_rec_old.h>

/** Builds a physical record out of a data tuple and
 stores it beginning from the start of the given buffer.
 @return pointer to the origin of physical record */
rec_t *rec_convert_dtuple_to_rec(
    byte *buf,                 /*!< in: start address of the
                               physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple,    /*!< in: data tuple */
    ulint n_ext)               /*!< in: number of
                               externally stored columns */
{
  rec_t *rec;

  ut_ad(buf != NULL);
  ut_ad(index != NULL);
  ut_ad(dtuple != NULL);
  ut_ad(dtuple_validate(dtuple));
  ut_ad(dtuple_check_typed(dtuple));

  if (dict_table_is_comp(index->table)) {
    rec = rec_convert_dtuple_to_rec_new(buf, index, dtuple);
  } else {
    rec = rec_convert_dtuple_to_rec_old(buf, dtuple, n_ext);
  }

#ifdef UNIV_DEBUG
  /* Can't check this if it's an index with instantly added columns,
  because if it comes from UPDATE, the fields of dtuple may be less than
  the on from index itself. */
  if (!index->has_instant_cols()) {
    mem_heap_t *heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    const ulint *offsets;
    ulint i;
    rec_offs_init(offsets_);

    offsets = rec_get_offsets(rec, index, offsets_, ULINT_UNDEFINED, &heap);
    ut_ad(rec_validate(rec, offsets));
    ut_ad(dtuple_get_n_fields(dtuple) == rec_offs_n_fields(offsets));

    for (i = 0; i < rec_offs_n_fields(offsets); i++) {
      ut_ad(!dfield_is_ext(dtuple_get_nth_field(dtuple, i)) ==
            !rec_offs_nth_extern(offsets, i));
    }

    if (UNIV_LIKELY_NULL(heap)) {
      mem_heap_free(heap);
    }
  }
#endif /* UNIV_DEBUG */
  return (rec);
}

