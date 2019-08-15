#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_tf_is_valid.h>


/** Validate both table flags and table flags2 and make sure they
are compatible.
@param[in]	flags	Table flags
@param[in]	flags2	Table flags2
@return true if valid. */
UNIV_INLINE
bool dict_tf2_is_valid(uint32_t flags, uint32_t flags2) {
  if (!dict_tf_is_valid(flags)) {
    return (false);
  }

  if ((flags2 & DICT_TF2_UNUSED_BIT_MASK) != 0) {
    return (false);
  }

  bool file_per_table = ((flags2 & DICT_TF2_USE_FILE_PER_TABLE) != 0);
  bool shared_space = DICT_TF_HAS_SHARED_SPACE(flags);
  if (file_per_table && shared_space) {
    return (false);
  }

  return (true);
}
