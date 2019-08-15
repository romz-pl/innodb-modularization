#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/disk/flags.h>

/** Validate a SYS_TABLES TYPE field and return it.
 @return Same as input after validating it as a SYS_TABLES TYPE field.
 If there is an error, return ULINT_UNDEFINED. */
UNIV_INLINE
uint32_t dict_sys_tables_type_validate(
    uint32_t type,   /*!< in: SYS_TABLES.TYPE */
    uint32_t n_cols) /*!< in: SYS_TABLES.N_COLS */
{
  uint32_t low_order_bit = DICT_TF_GET_COMPACT(type);
  uint32_t redundant = !(n_cols & DICT_N_COLS_COMPACT);
  uint32_t zip_ssize = DICT_TF_GET_ZIP_SSIZE(type);
  uint32_t atomic_blobs = DICT_TF_HAS_ATOMIC_BLOBS(type);
  uint32_t unused = DICT_TF_GET_UNUSED(type);

  /* The low order bit of SYS_TABLES.TYPE is always set to 1.
  If !atomic_blobs, this field is the same
  as dict_table_t::flags. Zero is not allowed here. */
  if (!low_order_bit) {
    return (UINT32_UNDEFINED);
  }

  if (redundant) {
    if (zip_ssize || atomic_blobs) {
      return (UINT32_UNDEFINED);
    }
  }

  /* Make sure there are no bits that we do not know about. */
  if (unused) {
    return (UINT32_UNDEFINED);
  }

  if (atomic_blobs) {
    /* ROW_FORMAT=COMPRESSED and ROW_FORMAT=DYNAMIC build on
    the page structure introduced for the COMPACT row format
    by allowing keys in secondary indexes to be made from
    data stored off-page in the clustered index.

    The DICT_N_COLS_COMPACT flag should be in N_COLS,
    but we already know that. */

  } else if (zip_ssize) {
    /* Antelope does not support COMPRESSED format. */
    return (UINT32_UNDEFINED);
  }

  if (zip_ssize) {
    /* COMPRESSED row format must have low_order_bit and
    atomic_blobs bits set and the DICT_N_COLS_COMPACT flag
    should be in N_COLS, but we already know about the
    low_order_bit and DICT_N_COLS_COMPACT flags. */
    if (!atomic_blobs) {
      return (UINT32_UNDEFINED);
    }

    /* Validate that the number is within allowed range. */
    if (zip_ssize > PAGE_ZIP_SSIZE_MAX) {
      return (UINT32_UNDEFINED);
    }
  }

  /* There is nothing to validate for the data_dir field.
  CREATE TABLE ... DATA DIRECTORY is supported for any row
  format, so the DATA_DIR flag is compatible with any other
  table flags. However, it is not used with TEMPORARY tables. */

  /* Return the validated SYS_TABLES.TYPE. */
  return (type);
}
