/*****************************************************************************

Copyright (c) 1994, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file rem/rec.h
 Record manager

 Created 5/30/1994 Heikki Tuuri
 *************************************************************************/

/** NOTE: The functions in this file should only use functions from
other files in library. The code in this file is used to make a library for
external tools. */

#ifndef rem_rec_h
#define rem_rec_h

#include <innodb/univ/univ.h>
#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/record/flag.h>

#include "dict0boot.h"
#include "dict0dict.h"


/** The following function determines the offsets to each field
 in the record. It can reuse a previously allocated array.
 Note that after instant ADD COLUMN, if this is a record
 from clustered index, fields in the record may be less than
 the fields defined in the clustered index. So the offsets
 size is allocated according to the clustered index fields.
 @return the new offsets */
ulint *rec_get_offsets_func(
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets,            /*!< in/out: array consisting of
                               offsets[0] allocated elements,
                               or an array from rec_get_offsets(),
                               or NULL */
    ulint n_fields,            /*!< in: maximum number of
                              initialized fields
                               (ULINT_UNDEFINED if all fields) */
#ifdef UNIV_DEBUG
    const char *file,  /*!< in: file name where called */
    ulint line,        /*!< in: line number where called */
#endif                 /* UNIV_DEBUG */
    mem_heap_t **heap) /*!< in/out: memory heap */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function determines the offsets to each field
 in the record.  It can reuse a previously allocated array. */
void rec_get_offsets_reverse(
    const byte *extra,         /*!< in: the extra bytes of a
                               compact record in reverse order,
                               excluding the fixed-size
                               REC_N_NEW_EXTRA_BYTES */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint node_ptr,            /*!< in: nonzero=node pointer,
                              0=leaf node */
    ulint *offsets);           /*!< in/out: array consisting of
                              offsets[0] allocated elements */




/** The following function is used to get the number of fields
in an old-style record. Have to consider the case that after
instant ADD COLUMN, this record may have less fields than
current index.
@param[in]	rec	physical record
@param[in]	index	index where the record resides
@return number of data fields */
UNIV_INLINE MY_ATTRIBUTE((warn_unused_result)) uint16_t
    rec_get_n_fields_old(const rec_t *rec, const dict_index_t *index) {
  uint16_t n = rec_get_n_fields_old_raw(rec);

  if (index->has_instant_cols()) {
    uint16_t n_uniq = dict_index_get_n_unique_in_tree_nonleaf(index);

    ut_ad(index->is_clustered());
    ut_ad(n <= dict_index_get_n_fields(index));
    ut_ad(n_uniq > 0);
    /* Only when it's infimum or supremum, n is 1.
    If n is exact n_uniq, this should be a record copied with prefix during
    search.
    And if it's node pointer, n is n_uniq + 1, which should be always less
    than the number of fields in any leaf page, even if the record in
    leaf page is before instant ADD COLUMN. This is because any record in
    leaf page must have at least n_uniq + 2 (system columns) fields */
    ut_ad(n == 1 || n >= n_uniq);
    ut_ad(static_cast<uint16_t>(dict_index_get_n_fields(index)) > n_uniq + 1);
    if (n > n_uniq + 1) {
#ifdef UNIV_DEBUG
      ulint rec_diff = dict_index_get_n_fields(index) - n;
      ulint col_diff = index->table->n_cols - index->table->n_instant_cols;
      ut_ad(rec_diff <= col_diff);
      if (n != dict_index_get_n_fields(index)) {
        ut_ad(index->has_instant_cols());
      }
#endif /* UNIV_DEBUG */
      n = static_cast<uint16_t>(dict_index_get_n_fields(index));
    }
  }

  return (n);
}

/** The following function is used to get the number of fields
 in a record. If it's REDUNDANT record, the returned number
 would be a logic one which considers the fact that after
 instant ADD COLUMN, some records may have less fields than
 index.
 @return number of data fields */
UNIV_INLINE
ulint rec_get_n_fields(const rec_t *rec,          /*!< in: physical record */
                       const dict_index_t *index) /*!< in: record descriptor */
{
  ut_ad(rec);
  ut_ad(index);

  if (!dict_table_is_comp(index->table)) {
    return (rec_get_n_fields_old(rec, index));
  }

  switch (rec_get_status(rec)) {
    case REC_STATUS_ORDINARY:
      return (dict_index_get_n_fields(index));
    case REC_STATUS_NODE_PTR:
      return (dict_index_get_n_unique_in_tree(index) + 1);
    case REC_STATUS_INFIMUM:
    case REC_STATUS_SUPREMUM:
      return (1);
    default:
      ut_error;
  }
}

/** Confirms the n_fields of the entry is sane with comparing the other
record in the same page specified
@param[in]	index	index
@param[in]	rec	record of the same page
@param[in]	entry	index entry
@return	true if n_fields is sane */
UNIV_INLINE
bool rec_n_fields_is_sane(dict_index_t *index, const rec_t *rec,
                          const dtuple_t *entry) {
  return (rec_get_n_fields(rec, index) == dtuple_get_n_fields(entry)
          /* a record for older SYS_INDEXES table
          (missing merge_threshold column) is acceptable. */
          || (index->table->id == DICT_INDEXES_ID &&
              rec_get_n_fields(rec, index) == dtuple_get_n_fields(entry) - 1));
}




/** Determine the offset of a specified field in the record, when this
field is a field added after an instant ADD COLUMN
@param[in]	index	Clustered index where the record resides
@param[in]	n	Nth field to get offset
@param[in]	offs	Last offset before current field
@return The offset of the specified field */
UNIV_INLINE
uint64_t rec_get_instant_offset(const dict_index_t *index, ulint n,
                                uint64_t offs) {
  ut_ad(index->has_instant_cols());

  ulint length;
  index->get_nth_default(n, &length);

  if (length == UNIV_SQL_NULL) {
    return (offs | REC_OFFS_SQL_NULL);
  } else {
    return (offs | REC_OFFS_DEFAULT);
  }
}

/** The following function determines the offsets to each field in the
 record.	 The offsets are written to a previously allocated array of
 ulint, where rec_offs_n_fields(offsets) has been initialized to the
 number of fields in the record.	 The rest of the array will be
 initialized by this function.  rec_offs_base(offsets)[0] will be set
 to the extra size (if REC_OFFS_COMPACT is set, the record is in the
 new format; if REC_OFFS_EXTERNAL is set, the record contains externally
 stored columns), and rec_offs_base(offsets)[1..n_fields] will be set to
 offsets past the end of fields 0..n_fields, or to the beginning of
 fields 1..n_fields+1.  When the high-order bit of the offset at [i+1]
 is set (REC_OFFS_SQL_NULL), the field i is NULL.  When the second
 high-order bit of the offset at [i+1] is set (REC_OFFS_EXTERNAL), the
 field i is being stored externally. */
void rec_init_offsets(const rec_t *rec,          /*!< in: physical record */
                      const dict_index_t *index, /*!< in: record descriptor */
                      ulint *offsets);           /*!< in/out: array of offsets;
                                                in: n=rec_offs_n_fields(offsets) */

#ifdef UNIV_DEBUG
/** Validates offsets returned by rec_get_offsets().
 @return true if valid */
UNIV_INLINE MY_ATTRIBUTE((warn_unused_result)) ibool rec_offs_validate(
    const rec_t *rec,          /*!< in: record or NULL */
    const dict_index_t *index, /*!< in: record descriptor or NULL */
    const ulint *offsets)      /*!< in: array returned by
                               rec_get_offsets() */
{
  ulint i = rec_offs_n_fields(offsets);
  ulint last = ULINT_MAX;
  ulint comp = *rec_offs_base(offsets) & REC_OFFS_COMPACT;

  if (rec) {
    ut_ad((ulint)rec == offsets[2]);
    if (!comp && index != nullptr) {
      ut_a(rec_get_n_fields_old(rec, index) >= i);
    }
  }
  if (index) {
    ulint max_n_fields;
    ut_ad((ulint)index == offsets[3]);
    ulint n_fields = dict_index_get_n_fields(index);
    ulint n_unique_in_tree = dict_index_get_n_unique_in_tree(index) + 1;
    max_n_fields = std::max(n_fields, n_unique_in_tree);
    if (!comp && rec != nullptr && rec_get_n_fields_old_raw(rec) < i) {
      ut_a(index->has_instant_cols());
    }

    if (comp && rec) {
      switch (rec_get_status(rec)) {
        case REC_STATUS_ORDINARY:
          break;
        case REC_STATUS_NODE_PTR:
          max_n_fields = dict_index_get_n_unique_in_tree(index) + 1;
          break;
        case REC_STATUS_INFIMUM:
        case REC_STATUS_SUPREMUM:
          max_n_fields = 1;
          break;
        default:
          ut_error;
      }
    }
    /* index->n_def == 0 for dummy indexes if !comp */
    ut_a(!comp || index->n_def);
    ut_a(!index->n_def || i <= max_n_fields);
  }
  while (i--) {
    ulint curr = rec_offs_base(offsets)[1 + i] & REC_OFFS_MASK;
    ut_a(curr <= last);
    last = curr;
  }
  return (TRUE);
}

/** Updates debug data in offsets, in order to avoid bogus
 rec_offs_validate() failures. */
UNIV_INLINE
void rec_offs_make_valid(
    const rec_t *rec,          /*!< in: record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets)            /*!< in: array returned by
                               rec_get_offsets() */
{
  ut_ad(rec);
  ut_ad(index);
  ut_ad(offsets);
  ut_ad(rec_get_n_fields(rec, index) >= rec_offs_n_fields(offsets));
  offsets[2] = (ulint)rec;
  offsets[3] = (ulint)index;
}

/** Check if the given two record offsets are identical.
@param[in]  offsets1  field offsets of a record
@param[in]  offsets2  field offsets of a record
@return true if they are identical, false otherwise. */
bool rec_offs_cmp(ulint *offsets1, ulint *offsets2);

/** Print the record offsets.
@param[in]    out         the output stream to which offsets are printed.
@param[in]    offsets     the field offsets of the record.
@return the output stream. */
std::ostream &rec_offs_print(std::ostream &out, const ulint *offsets);
#else
#define rec_offs_make_valid(rec, index, offsets) ((void)0)
#endif /* UNIV_DEBUG */


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

/** Determines the information about null bytes and variable length bytes
for a new style record
@param[in]	rec		physical record
@param[in]	index		index where the record resides
@param[out]	nulls		the start of null bytes
@param[out]	lens		the start of variable length bytes
@param[out]	n_null		number of null fields
@return	the number of fields which are inlined of the record */
UNIV_INLINE
uint16_t rec_init_null_and_len_comp(const rec_t *rec, const dict_index_t *index,
                                    const byte **nulls, const byte **lens,
                                    uint16_t *n_null) {
  uint16_t non_default_fields =
      static_cast<uint16_t>(dict_index_get_n_fields(index));

  *nulls = rec - (REC_N_NEW_EXTRA_BYTES + 1);

  if (!index->has_instant_cols()) {
    *n_null = index->n_nullable;
  } else if (rec_get_instant_flag_new(rec)) {
    /* Row inserted after first instant ADD COLUMN */
    uint16_t length;
    non_default_fields =
        rec_get_n_fields_instant(rec, REC_N_NEW_EXTRA_BYTES, &length);
    ut_ad(length == 1 || length == 2);

    *nulls -= length;
    *n_null = index->get_n_nullable_before(non_default_fields);
  } else {
    /* Row inserted before first instant ADD COLUMN */
    *n_null = index->n_instant_nullable;
    non_default_fields = index->get_instant_fields();
  }

  *lens = *nulls - UT_BITS_IN_BYTES(*n_null);

  return (non_default_fields);
}

/** Determine the offset to each field in a leaf-page record
 in ROW_FORMAT=COMPACT.  This is a special case of
 rec_init_offsets() and rec_get_offsets_func(). */
UNIV_INLINE
void rec_init_offsets_comp_ordinary(
    const rec_t *rec,          /*!< in: physical record in
                               ROW_FORMAT=COMPACT */
    bool temp,                 /*!< in: whether to use the
                               format for temporary files in
                               index creation */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets)            /*!< in/out: array of offsets;
                               in: n=rec_offs_n_fields(offsets) */
{
  uint16_t i = 0;
  ulint offs = 0;
  ulint any_ext = 0;
  uint16_t n_null = 0;
  const byte *nulls = nullptr;
  const byte *lens = nullptr;
  ulint null_mask = 1;
  uint16_t non_default_fields = 0;

#ifdef UNIV_DEBUG
  /* We cannot invoke rec_offs_make_valid() here if temp=true.
  Similarly, rec_offs_validate() will fail in that case, because
  it invokes rec_get_status(). */
  offsets[2] = (ulint)rec;
  offsets[3] = (ulint)index;
#endif /* UNIV_DEBUG */

  if (temp) {
    non_default_fields =
        rec_init_null_and_len_temp(rec, index, &nulls, &lens, &n_null);
  } else {
    non_default_fields =
        rec_init_null_and_len_comp(rec, index, &nulls, &lens, &n_null);
  }

  ut_ad(temp || dict_table_is_comp(index->table));

  if (temp && dict_table_is_comp(index->table)) {
    /* No need to do adjust fixed_len=0. We only need to
    adjust it for ROW_FORMAT=REDUNDANT. */
    temp = false;
  }

  /* read the lengths of fields 0..n */
  do {
    const dict_field_t *field = index->get_field(i);
    const dict_col_t *col = field->col;
    uint64_t len;

    if (i >= non_default_fields) {
      ut_ad(index->has_instant_cols());

      len = rec_get_instant_offset(index, i, offs);

      goto resolved;
    }

    if (!(col->prtype & DATA_NOT_NULL)) {
      /* nullable field => read the null flag */
      ut_ad(n_null--);

      if (UNIV_UNLIKELY(!(byte)null_mask)) {
        nulls--;
        null_mask = 1;
      }

      if (*nulls & null_mask) {
        null_mask <<= 1;
        /* No length is stored for NULL fields.
        We do not advance offs, and we set
        the length to zero and enable the
        SQL NULL flag in offsets[]. */
        len = offs | REC_OFFS_SQL_NULL;
        goto resolved;
      }
      null_mask <<= 1;
    }

    if (!field->fixed_len || (temp && !col->get_fixed_size(temp))) {
      ut_ad(col->mtype != DATA_POINT);
      /* Variable-length field: read the length */
      len = *lens--;
      /* If the maximum length of the field is up
      to 255 bytes, the actual length is always
      stored in one byte. If the maximum length is
      more than 255 bytes, the actual length is
      stored in one byte for 0..127.  The length
      will be encoded in two bytes when it is 128 or
      more, or when the field is stored externally. */
      if (DATA_BIG_COL(col)) {
        if (len & 0x80) {
          /* 1exxxxxxx xxxxxxxx */
          len <<= 8;
          len |= *lens--;

          offs += len & 0x3fff;
          if (UNIV_UNLIKELY(len & 0x4000)) {
            ut_ad(index->is_clustered());
            any_ext = REC_OFFS_EXTERNAL;
            len = offs | REC_OFFS_EXTERNAL;
          } else {
            len = offs;
          }

          goto resolved;
        }
      }

      len = offs += len;
    } else {
      len = offs += field->fixed_len;
    }
  resolved:
    rec_offs_base(offsets)[i + 1] = len;
  } while (++i < rec_offs_n_fields(offsets));

  *rec_offs_base(offsets) = (rec - (lens + 1)) | REC_OFFS_COMPACT | any_ext;
}

#endif
