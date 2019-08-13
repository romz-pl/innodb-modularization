#pragma once

#include <innodb/univ/univ.h>

#include <innodb/data_types/dtype_get_fixed_size_low.h>
#include <innodb/data_types/dtype_get_max_size_low.h>
#include <innodb/data_types/dtype_get_min_size_low.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/data_types/spatial_status_t.h>
#include <innodb/dict_mem/dict_col_default_t.h>
#include <innodb/memory/mem_heap_t.h>

/** Data structure for a column in a table */
struct dict_col_t {
  /*----------------------*/
  /** The following are copied from dtype_t,
  so that all bit-fields can be packed tightly. */
  /* @{ */

  /** Default value when this column was added instantly.
  If this is not a instantly added column then this is nullptr. */
  dict_col_default_t *instant_default;

  unsigned prtype : 32; /*!< precise type; MySQL data
                        type, charset code, flags to
                        indicate nullability,
                        signedness, whether this is a
                        binary string, whether this is
                        a true VARCHAR where MySQL
                        uses 2 bytes to store the length */
  unsigned mtype : 8;   /*!< main data type */

  /* the remaining fields do not affect alphabetical ordering: */

  unsigned len : 16; /*!< length; for MySQL data this
                     is field->pack_length(),
                     except that for a >= 5.0.3
                     type true VARCHAR this is the
                     maximum byte length of the
                     string data (in addition to
                     the string, MySQL uses 1 or 2
                     bytes to store the string length) */

  unsigned mbminmaxlen : 5; /*!< minimum and maximum length of a
                            character, in bytes;
                            DATA_MBMINMAXLEN(mbminlen,mbmaxlen);
                            mbminlen=DATA_MBMINLEN(mbminmaxlen);
                            mbmaxlen=DATA_MBMINLEN(mbminmaxlen) */
  /*----------------------*/
  /* End of definitions copied from dtype_t */
  /* @} */

  unsigned ind : 10;        /*!< table column position
                            (starting from 0) */
  unsigned ord_part : 1;    /*!< nonzero if this column
                            appears in the ordering fields
                            of an index */
  unsigned max_prefix : 12; /*!< maximum index prefix length on
                            this column. Our current max limit is
                            3072 (REC_VERSION_56_MAX_INDEX_COL_LEN)
                            bytes. */

  /** Returns the minimum size of the column.
  @return minimum size */
  ulint get_min_size() const {
    return (dtype_get_min_size_low(mtype, prtype, len, mbminmaxlen));
  }

  /** Returns the maximum size of the column.
  @return maximum size */
  ulint get_max_size() const { return (dtype_get_max_size_low(mtype, len)); }

  /** Check if a column is a virtual column
  @return true if it is a virtual column, false otherwise */
  bool is_virtual() const { return (prtype & DATA_VIRTUAL); }

  /** Check if a column is nullable
  @return true if it is nullable, otherwise false */
  bool is_nullable() const { return ((prtype & DATA_NOT_NULL) == 0); }

  /** Gets the column data type.
  @param[out] type	data type */
  void copy_type(dtype_t *type) const {
    ut_ad(type != NULL);

    type->mtype = mtype;
    type->prtype = prtype;
    type->len = len;
    type->mbminmaxlen = mbminmaxlen;
  }

  /** Gets the minimum number of bytes per character.
  @return minimum multi-byte char size, in bytes */
  ulint get_mbminlen() const { return (DATA_MBMINLEN(mbminmaxlen)); }

  /** Gets the maximum number of bytes per character.
  @return maximum multi-byte char size, in bytes */
  ulint get_mbmaxlen() const { return (DATA_MBMAXLEN(mbminmaxlen)); }

  /** Sets the minimum and maximum number of bytes per character.
  @param[in] mbminlen	minimum multi byte character size, in bytes
  @param[in] mbmaxlen	mAXimum multi-byte character size, in bytes */
  void set_mbminmaxlen(ulint mbminlen, ulint mbmaxlen) {
    ut_ad(mbminlen < DATA_MBMAX);
    ut_ad(mbmaxlen < DATA_MBMAX);
    ut_ad(mbminlen <= mbmaxlen);

    mbminmaxlen = DATA_MBMINMAXLEN(mbminlen, mbmaxlen);
  }

  /** Returns the size of a fixed size column, 0 if not a fixed size column.
  @param[in] comp		nonzero=ROW_FORMAT=COMPACT
  @return fixed size, or 0 */
  ulint get_fixed_size(ulint comp) const {
    return (dtype_get_fixed_size_low(mtype, prtype, len, mbminmaxlen, comp));
  }

  /** Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a column.
  For fixed length types it is the fixed length of the type, otherwise 0.
  @param[in] comp		nonzero=ROW_FORMAT=COMPACT
  @return SQL null storage size in ROW_FORMAT=REDUNDANT */
  ulint get_null_size(ulint comp) const { return (get_fixed_size(comp)); }

  /** Check whether the col is used in spatial index or regular index.
  @return spatial status */
  spatial_status_t get_spatial_status() const {
    spatial_status_t spatial_status = SPATIAL_NONE;

    /* Column is not a part of any index. */
    if (!ord_part) {
      return (spatial_status);
    }

    if (DATA_GEOMETRY_MTYPE(mtype)) {
      if (max_prefix == 0) {
        spatial_status = SPATIAL_ONLY;
      } else {
        /* Any regular index on a geometry column
        should have a prefix. */
        spatial_status = SPATIAL_MIXED;
      }
    }

    return (spatial_status);
  }

  /** Set default value
  @param[in]	value	Default value
  @param[in]	length	Default value length
  @param[in,out]	heap	Heap to allocate memory */
  void set_default(const byte *value, size_t length, mem_heap_t *heap);

#ifdef UNIV_DEBUG
  /** Assert that a column and a data type match.
  param[in] type		data type
  @return true */
  bool assert_equal(const dtype_t *type) const {
    ut_ad(type);

    ut_ad(mtype == type->mtype);
    ut_ad(prtype == type->prtype);
    // ut_ad(col->len == type->len);
#ifndef UNIV_HOTBACKUP
    ut_ad(mbminmaxlen == type->mbminmaxlen);
#endif /* !UNIV_HOTBACKUP */

    return true;
  }
#endif /* UNIV_DEBUG */
};
