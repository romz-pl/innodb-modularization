#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/dict_mem/rec_format_t.h>
#include <innodb/assert/assert.h>

/** Set the various values in a dict_table_t::flags pointer.
@param[in,out]	flags		Pointer to a 4 byte Table Flags
@param[in]	format		File Format
@param[in]	zip_ssize	Zip Shift Size
@param[in]	use_data_dir	Table uses DATA DIRECTORY
@param[in]	shared_space	Table uses a General Shared Tablespace */
UNIV_INLINE
void dict_tf_set(uint32_t *flags, rec_format_t format, ulint zip_ssize,
                 bool use_data_dir, bool shared_space) {
  switch (format) {
    case REC_FORMAT_REDUNDANT:
      *flags = 0;
      ut_ad(zip_ssize == 0);
      break;
    case REC_FORMAT_COMPACT:
      *flags = DICT_TF_COMPACT;
      ut_ad(zip_ssize == 0);
      break;
    case REC_FORMAT_COMPRESSED:
      *flags = DICT_TF_COMPACT | (1 << DICT_TF_POS_ATOMIC_BLOBS) |
               (zip_ssize << DICT_TF_POS_ZIP_SSIZE);
      break;
    case REC_FORMAT_DYNAMIC:
      *flags = DICT_TF_COMPACT | (1 << DICT_TF_POS_ATOMIC_BLOBS);
      ut_ad(zip_ssize == 0);
      break;
  }

  if (use_data_dir) {
    *flags |= (1 << DICT_TF_POS_DATA_DIR);
  }

  if (shared_space) {
    *flags |= (1 << DICT_TF_POS_SHARED_SPACE);
  }
}
