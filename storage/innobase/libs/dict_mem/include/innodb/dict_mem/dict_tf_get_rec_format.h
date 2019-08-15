#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_tf_is_valid.h>
#include <innodb/dict_mem/rec_format_t.h>
#include <innodb/assert/assert.h>

/** Determine the page format from dict_table_t::flags
 The low order bit will be zero for REDUNDANT and 1 for COMPACT. For any
 other row_format, flags is nonzero and DICT_TF_COMPACT will also be set.
 @return file format version */
UNIV_INLINE
rec_format_t dict_tf_get_rec_format(
    uint32_t flags) /*!< in: dict_table_t::flags */
{
  ut_a(dict_tf_is_valid(flags));

  if (!DICT_TF_GET_COMPACT(flags)) {
    return (REC_FORMAT_REDUNDANT);
  }

  if (!DICT_TF_HAS_ATOMIC_BLOBS(flags)) {
    return (REC_FORMAT_COMPACT);
  }

  if (DICT_TF_GET_ZIP_SSIZE(flags)) {
    return (REC_FORMAT_COMPRESSED);
  }

  return (REC_FORMAT_DYNAMIC);
}
