#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_tf_is_valid.h>
#include <innodb/assert/assert.h>

/** Convert a 32 bit integer table flags to the 32bit integer that is written
 to a SYS_TABLES.TYPE field. The following chart shows the translation of
 the low order bit.  Other bits are the same.
 ========================= Low order bit ==========================
                     | REDUNDANT | COMPACT | COMPRESSED and DYNAMIC
 dict_table_t::flags |     0     |    1    |     1
 SYS_TABLES.TYPE     |     1     |    1    |     1
 ==================================================================
 @return ulint containing SYS_TABLES.TYPE */
UNIV_INLINE
ulint dict_tf_to_sys_tables_type(uint32_t flags) /*!< in: dict_table_t::flags */
{
  ulint type;

  ut_a(dict_tf_is_valid(flags));

  /* Adjust bit zero. It is always 1 in SYS_TABLES.TYPE */
  type = 1;

  /* ZIP_SSIZE, ATOMIC_BLOBS & DATA_DIR are the same. */
  type |= flags & (DICT_TF_MASK_ZIP_SSIZE | DICT_TF_MASK_ATOMIC_BLOBS |
                   DICT_TF_MASK_DATA_DIR | DICT_TF_MASK_SHARED_SPACE);

  return (type);
}
