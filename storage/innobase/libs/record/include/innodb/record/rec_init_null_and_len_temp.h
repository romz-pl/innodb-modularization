#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_get_instant_flag_new_temp.h>
#include <innodb/record/rec_get_n_fields_instant.h>
#include <innodb/bit/UT_BITS_IN_BYTES.h>

/** Determines the information about null bytes and variable length bytes
for a new-style temporary record
@param[in]	rec		physical record
@param[in]	index		index where the record resides
@param[out]	nulls		the start of null bytes
@param[out]	lens		the start of variable length bytes
@param[out]	n_null		number of null fields
@return	the number of fields which are inlined of the record */
UNIV_INLINE
uint16_t rec_init_null_and_len_temp(const rec_t *rec, const dict_index_t *index,
                                    const byte **nulls, const byte **lens,
                                    uint16_t *n_null) {
  uint16_t non_default_fields =
      static_cast<uint16_t>(dict_index_get_n_fields(index));

  *nulls = rec - 1;

  if (index->has_instant_cols() && dict_table_is_comp(index->table)) {
    *nulls -= REC_N_TMP_EXTRA_BYTES;
  }

  if (!index->has_instant_cols() || !dict_table_is_comp(index->table)) {
    *n_null = index->n_nullable;
  } else if (rec_get_instant_flag_new_temp(rec)) {
    ut_ad(index->has_instant_cols());

    uint16_t length;
    non_default_fields =
        rec_get_n_fields_instant(rec, REC_N_TMP_EXTRA_BYTES, &length);
    ut_ad(length == 1 || length == 2);

    *nulls -= length;
    *n_null = index->get_n_nullable_before(non_default_fields);
  } else {
    *n_null = index->n_instant_nullable;
    non_default_fields = index->get_instant_fields();
  }

  *lens = *nulls - UT_BITS_IN_BYTES(*n_null);

  return (non_default_fields);
}
