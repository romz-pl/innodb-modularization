#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_tf_is_valid.h>
#include <innodb/assert/assert.h>

/** Initialize a dict_table_t::flags pointer.
@param[in]	compact		Table uses Compact or greater
@param[in]	zip_ssize	Zip Shift Size (log 2 minus 9)
@param[in]	atomic_blobs	Table uses Compressed or Dynamic
@param[in]	data_dir	Table uses DATA DIRECTORY
@param[in]	shared_space	Table uses a General Shared Tablespace */
UNIV_INLINE
uint32_t dict_tf_init(bool compact, ulint zip_ssize, bool atomic_blobs,
                      bool data_dir, bool shared_space) {
  uint32_t flags = 0;

  if (compact) {
    flags |= DICT_TF_COMPACT;
  }

  if (zip_ssize) {
    flags |= (zip_ssize << DICT_TF_POS_ZIP_SSIZE);
  }

  if (atomic_blobs) {
    flags |= (1 << DICT_TF_POS_ATOMIC_BLOBS);
  }

  if (data_dir) {
    flags |= (1 << DICT_TF_POS_DATA_DIR);
  }

  if (shared_space) {
    flags |= (1 << DICT_TF_POS_SHARED_SPACE);
  }

  ut_ad(dict_tf_is_valid(flags));

  return (flags);
}
