#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/disk/flags.h>

/** Validate the table flags.
@param[in]	flags	Table flags
@return true if valid. */
UNIV_INLINE
bool dict_tf_is_valid(uint32_t flags) {
  bool compact = DICT_TF_GET_COMPACT(flags);
  uint32_t zip_ssize = DICT_TF_GET_ZIP_SSIZE(flags);
  bool atomic_blobs = DICT_TF_HAS_ATOMIC_BLOBS(flags);
  bool data_dir = DICT_TF_HAS_DATA_DIR(flags);
  bool shared_space = DICT_TF_HAS_SHARED_SPACE(flags);
  uint32_t unused = DICT_TF_GET_UNUSED(flags);

  /* Make sure there are no bits that we do not know about. */
  if (unused != 0) {
    return (false);
  }

  if (atomic_blobs) {
    /* ROW_FORMAT=COMPRESSED and ROW_FORMAT=DYNAMIC both use
    atomic_blobs, which build on the page structure introduced
    for the COMPACT row format by allowing keys in secondary
    indexes to be made from data stored off-page in the
    clustered index. */

    if (!compact) {
      return (false);
    }

  } else if (zip_ssize) {
    /* Antelope does not support COMPRESSED row format. */
    return (false);
  }

  if (zip_ssize) {
    /* COMPRESSED row format must have compact and atomic_blobs
    bits set and validate the number is within allowed range. */

    if (!compact || !atomic_blobs || zip_ssize > PAGE_ZIP_SSIZE_MAX) {
      return (false);
    }
  }

  /* HAS_DATA_DIR and SHARED_SPACE are mutually exclusive. */
  if (data_dir && shared_space) {
    return (false);
  }

  return (true);
}
