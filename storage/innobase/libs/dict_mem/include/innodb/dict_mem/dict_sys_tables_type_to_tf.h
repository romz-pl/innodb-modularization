#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/assert/assert.h>

/** Convert a 32 bit integer from SYS_TABLES.TYPE to dict_table_t::flags
 The following chart shows the translation of the low order bit.
 Other bits are the same.
 ========================= Low order bit ==========================
                     | REDUNDANT | COMPACT | COMPRESSED and DYNAMIC
 SYS_TABLES.TYPE     |     1     |    1    |     1
 dict_table_t::flags |     0     |    1    |     1
 ==================================================================
 @return ulint containing SYS_TABLES.TYPE */
UNIV_INLINE
uint32_t dict_sys_tables_type_to_tf(
    uint32_t type,   /*!< in: SYS_TABLES.TYPE field */
    uint32_t n_cols) /*!< in: SYS_TABLES.N_COLS field */
{
  uint32_t flags;
  uint32_t redundant = !(n_cols & DICT_N_COLS_COMPACT);

  /* Adjust bit zero. */
  flags = redundant ? 0 : 1;

  /* ZIP_SSIZE, ATOMIC_BLOBS & DATA_DIR are the same. */
  flags |= type & (DICT_TF_MASK_ZIP_SSIZE | DICT_TF_MASK_ATOMIC_BLOBS |
                   DICT_TF_MASK_DATA_DIR | DICT_TF_MASK_SHARED_SPACE);

  ut_ad(!DICT_TF_GET_ZIP_SSIZE(flags) || DICT_TF_HAS_ATOMIC_BLOBS(flags));

  return (flags);
}
