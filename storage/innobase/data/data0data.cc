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

/** @file data/data0data.cc
 SQL data field and tuple

 Created 5/30/1994 Heikki Tuuri
 *************************************************************************/

#include <innodb/data_types/dfield_set_ext.h>
#include <innodb/data_types/dtype_get_mbmaxlen.h>
#include <innodb/data_types/dtype_get_mbminlen.h>
#include <innodb/logger/warn.h>
#include <innodb/print/ut_print_buf_hex.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/data_types/dtype_get_prtype.h>
#include <innodb/data_types/dtype_get_len.h>
#include <innodb/data_types/dtype_get_mtype.h>
#include <innodb/data_types/data_error.h>
#include <innodb/data_types/dfield_check_typed_no_assert.h>
#include <innodb/data_types/dtuple_check_typed_no_assert.h>

#include <sys/types.h>
#include <new>

#include "data0data.h"

#ifndef UNIV_HOTBACKUP
#include "row0upd.h"

#endif /* !UNIV_HOTBACKUP */

void row_mysql_pad_col(ulint mbminlen, byte *pad, ulint len);






#ifndef UNIV_HOTBACKUP



/** Moves parts of long fields in entry to the big record vector so that
 the size of tuple drops below the maximum record size allowed in the
 database. Moves data only from those fields which are not necessary
 to determine uniquely the insertion place of the tuple in the index.
 @return own: created big record vector, NULL if we are not able to
 shorten the entry enough, i.e., if there are too many fixed-length or
 short fields in entry or the index is clustered */
big_rec_t *dtuple_convert_big_rec(dict_index_t *index, /*!< in: index */
                                  upd_t *upd,      /*!< in/out: update vector */
                                  dtuple_t *entry, /*!< in/out: index entry */
                                  ulint *n_ext)    /*!< in/out: number of
                                                   externally stored columns */
{
  DBUG_ENTER("dtuple_convert_big_rec");

  mem_heap_t *heap;
  big_rec_t *vector;
  dfield_t *dfield;
  dict_field_t *ifield;
  ulint size;
  ulint n_fields;
  ulint local_len;
  ulint local_prefix_len;

  if (!index->is_clustered()) {
    DBUG_RETURN(NULL);
  }

  if (!dict_table_has_atomic_blobs(index->table)) {
    /* up to MySQL 5.1: store a 768-byte prefix locally */
    local_len = BTR_EXTERN_FIELD_REF_SIZE + DICT_ANTELOPE_MAX_INDEX_COL_LEN;
  } else {
    /* new-format table: do not store any BLOB prefix locally */
    local_len = BTR_EXTERN_FIELD_REF_SIZE;
  }

  ut_a(dtuple_check_typed_no_assert(entry));

  size = rec_get_converted_size(index, entry, *n_ext);

  if (UNIV_UNLIKELY(size > 1000000000)) {
    ib::warn(ER_IB_MSG_159) << "Tuple size is very big: " << size;
    fputs("InnoDB: Tuple contents: ", stderr);
    dtuple_print(stderr, entry);
    putc('\n', stderr);
  }

  heap = mem_heap_create(
      size + dtuple_get_n_fields(entry) * sizeof(big_rec_field_t) + 1000);

  vector = big_rec_t::alloc(heap, dtuple_get_n_fields(entry));

  /* Decide which fields to shorten: the algorithm is to look for
  a variable-length field that yields the biggest savings when
  stored externally */

  n_fields = 0;

  while (page_zip_rec_needs_ext(rec_get_converted_size(index, entry, *n_ext),
                                dict_table_is_comp(index->table),
                                dict_index_get_n_fields(index),
                                dict_table_page_size(index->table))) {
    ulint i;
    ulint longest = 0;
    ulint longest_i = ULINT_MAX;
    byte *data;
    upd_field_t *uf = nullptr;

    for (i = dict_index_get_n_unique_in_tree(index);
         i < dtuple_get_n_fields(entry); i++) {
      ulint savings;

      dfield = dtuple_get_nth_field(entry, i);
      ifield = index->get_field(i);

      /* Skip fixed-length, NULL, externally stored,
      or short columns */

      if (ifield->fixed_len || dfield_is_null(dfield) ||
          dfield_is_ext(dfield) || dfield_get_len(dfield) <= local_len ||
          dfield_get_len(dfield) <= BTR_EXTERN_LOCAL_STORED_MAX_SIZE) {
        goto skip_field;
      }

      savings = dfield_get_len(dfield) - local_len;

      /* Check that there would be savings */
      if (longest >= savings) {
        goto skip_field;
      }

      /* In DYNAMIC and COMPRESSED format, store
      locally any non-BLOB columns whose maximum
      length does not exceed 256 bytes.  This is
      because there is no room for the "external
      storage" flag when the maximum length is 255
      bytes or less. This restriction trivially
      holds in REDUNDANT and COMPACT format, because
      there we always store locally columns whose
      length is up to local_len == 788 bytes.
      @see rec_init_offsets_comp_ordinary */
      if (!DATA_BIG_COL(ifield->col)) {
        goto skip_field;
      }

      longest_i = i;
      longest = savings;

    skip_field:
      continue;
    }

    if (!longest) {
      /* Cannot shorten more */

      mem_heap_free(heap);

      DBUG_RETURN(NULL);
    }

    /* Move data from field longest_i to big rec vector.

    We store the first bytes locally to the record. Then
    we can calculate all ordering fields in all indexes
    from locally stored data. */

    dfield = dtuple_get_nth_field(entry, longest_i);

    ifield = index->get_field(longest_i);
    local_prefix_len = local_len - BTR_EXTERN_FIELD_REF_SIZE;

    big_rec_field_t big_rec(
        longest_i, dfield_get_len(dfield) - local_prefix_len,
        static_cast<char *>(dfield_get_data(dfield)) + local_prefix_len);

    /* Allocate the locally stored part of the column. */
    data = static_cast<byte *>(mem_heap_alloc(heap, local_len));

    /* Copy the local prefix (including LOB pointer). */
    memcpy(data, dfield_get_data(dfield), local_len);

    /* Clear the extern field reference (BLOB pointer). */
    memset(data + local_prefix_len, 0, BTR_EXTERN_FIELD_REF_SIZE);

    if (upd != nullptr &&
        ((uf = upd->get_field_by_field_no(longest_i, index)) != nullptr)) {
      /* When the externally stored LOB is going to be
      updated, the old LOB reference (BLOB pointer) can be
      used to access the old LOB object. So copy the LOB
      reference here. */

      if (dfield_is_ext(&uf->old_val)) {
        byte *field_ref = static_cast<byte *>(dfield_get_data(&uf->old_val)) +
                          local_prefix_len;
        memcpy(data + local_prefix_len, field_ref, lob::ref_t::SIZE);
      }
    }

#if 0
        /* The following would fail the Valgrind checks in
        page_cur_insert_rec_low() and page_cur_insert_rec_zip().
        The BLOB pointers in the record will be initialized after
        the record and the BLOBs have been written. */
        UNIV_MEM_ALLOC(data + local_prefix_len,
                   BTR_EXTERN_FIELD_REF_SIZE);
#endif

    dfield_set_data(dfield, data, local_len);
    dfield_set_ext(dfield);

    n_fields++;
    (*n_ext)++;
    ut_ad(n_fields < dtuple_get_n_fields(entry));

    if (upd && !upd->is_modified(longest_i)) {
      DEBUG_SYNC_C("ib_mv_nonupdated_column_offpage");

      upd_field_t upd_field;
      upd_field.field_no = longest_i;
      upd_field.orig_len = 0;
      upd_field.exp = NULL;
      upd_field.old_v_val = NULL;
      upd_field.ext_in_old = dfield_is_ext(dfield);
      dfield_copy(&upd_field.new_val, dfield->clone(upd->heap));
      upd->append(upd_field);
      ut_ad(upd->is_modified(longest_i));

      ut_ad(upd_field.new_val.len >= BTR_EXTERN_FIELD_REF_SIZE);
      ut_ad(upd_field.new_val.len == local_len);
      ut_ad(upd_field.new_val.len == dfield_get_len(dfield));
    }

    if (upd == nullptr) {
      big_rec.ext_in_old = false;
    } else {
      upd_field_t *uf = upd->get_field_by_field_no(longest_i, index);
      ut_ad(uf != nullptr);
      big_rec.ext_in_old = uf->ext_in_old;
    }

    big_rec.ext_in_new = true;
    vector->append(big_rec);
  }

  ut_ad(n_fields == vector->n_fields);
  DBUG_RETURN(vector);
}

/** Puts back to entry the data stored in vector. Note that to ensure the
 fields in entry can accommodate the data, vector must have been created
 from entry with dtuple_convert_big_rec. */
void dtuple_convert_back_big_rec(
    dict_index_t *index MY_ATTRIBUTE((unused)), /*!< in: index */
    dtuple_t *entry,   /*!< in: entry whose data was put to vector */
    big_rec_t *vector) /*!< in, own: big rec vector; it is
                       freed in this function */
{
  big_rec_field_t *b = vector->fields;
  const big_rec_field_t *const end = b + vector->n_fields;

  for (; b < end; b++) {
    dfield_t *dfield;
    ulint local_len;

    dfield = dtuple_get_nth_field(entry, b->field_no);
    local_len = dfield_get_len(dfield);

    ut_ad(dfield_is_ext(dfield));
    ut_ad(local_len >= BTR_EXTERN_FIELD_REF_SIZE);

    local_len -= BTR_EXTERN_FIELD_REF_SIZE;

    /* Only in REDUNDANT and COMPACT format, we store
    up to DICT_ANTELOPE_MAX_INDEX_COL_LEN (768) bytes
    locally */
    ut_ad(local_len <= DICT_ANTELOPE_MAX_INDEX_COL_LEN);

    dfield_set_data(dfield, (char *)b->data - local_len, b->len + local_len);
  }

  mem_heap_free(vector->heap);
}





uint32_t dfield_t::lob_version() const {
  ut_ad(ext);
  byte *field_ref = blobref();

  lob::ref_t ref(field_ref);
  return (ref.version());
}

/** Adjust and(or) set virtual column value which is read from undo
or online DDL log
@param[in]	vcol		virtual column definition
@param[in]	comp		true if compact format
@param[in]	field		virtual column value
@param[in]	len		value length
@param[in,out]	heap		memory heap to keep value when necessary */
void dfield_t::adjust_v_data_mysql(const dict_v_col_t *vcol, bool comp,
                                   const byte *field, ulint len,
                                   mem_heap_t *heap) {
  ulint mtype;
  const byte *data = field;

  ut_ad(heap != nullptr);

  mtype = type.mtype;

  if (mtype != DATA_MYSQL) {
    dfield_set_data(this, field, len);
    return;
  }

  /* Adjust the value if the data type is DATA_MYSQL, either
  adding or striping trailing spaces when necessary. This may happen
  in the scenario where there is an ALTER TABLE changing table's
  row format from compact to non-compact or vice versa, and there
  is also concurrent INSERT to this table. The log for the data could
  be in different format from the final format, which should be adjusted.
  Refer to row_mysql_store_col_in_innobase_format() too. */
  if (comp && len == vcol->m_col.len && dtype_get_mbminlen(&type) == 1 &&
      dtype_get_mbmaxlen(&type) > 1) {
    /* A full length record, which is of multibyte
    charsets and recorded because old table is non-compact.
    However, in compact table, no trailing spaces. */
    ulint n_chars;

    ut_a(!(dtype_get_len(&type) % dtype_get_mbmaxlen(&type)));

    n_chars = dtype_get_len(&type) / dtype_get_mbmaxlen(&type);

    while (len > n_chars && data[len - 1] == 0x20) {
      --len;
    }
  } else if (!comp && len < vcol->m_col.len && dtype_get_mbminlen(&type) == 1) {
    /* A not full length record from compact table, so have to
    add trailing spaces. */
    byte *v_data =
        reinterpret_cast<byte *>(mem_heap_alloc(heap, vcol->m_col.len));

    memcpy(v_data, field, len);
    row_mysql_pad_col(1, v_data + len, vcol->m_col.len - len);

    data = v_data;
    len = vcol->m_col.len;
  }

  dfield_set_data(this, data, len);
}

/** Print the dfield_t object into the given output stream.
@param[in]	out	the output stream.
@return	the ouput stream. */
std::ostream &dfield_t::print(std::ostream &out) const {
  out << "[dfield_t: data=" << (void *)data << ", ext=" << ext << " ";

  if (dfield_is_ext(this)) {
    byte *tmp = static_cast<byte *>(data);
    lob::ref_t ref(tmp + len - lob::ref_t::SIZE);
    out << ref;
  }

  out << ", spatial_status=" << spatial_status << ", len=" << len << ", type="
      << "]";

  return (out);
}

#ifdef UNIV_DEBUG
/** Print the big_rec_field_t object into the given output stream.
@param[in]	out	the output stream.
@return	the ouput stream. */
std::ostream &big_rec_field_t::print(std::ostream &out) const {
  out << "[big_rec_field_t: field_no=" << field_no << ", len=" << len
      << ", data=" << PrintBuffer(data, len) << ", ext_in_old=" << ext_in_old
      << ", ext_in_new=" << ext_in_new << "]";
  return (out);
}

/** Print the current object into the given output stream.
@param[in]	out	the output stream.
@return	the ouput stream. */
std::ostream &big_rec_t::print(std::ostream &out) const {
  out << "[big_rec_t: capacity=" << capacity << ", n_fields=" << n_fields
      << " ";
  for (ulint i = 0; i < n_fields; ++i) {
    out << fields[i];
  }
  out << "]";
  return (out);
}
#endif /* UNIV_DEBUG */



/** Ignore trailing default fields if this is a tuple from instant index
@param[in]	index		clustered index object for this tuple */
void dtuple_t::ignore_trailing_default(const dict_index_t *index) {
  if (!index->has_instant_cols()) {
    return;
  }

  /* It's necessary to check all the fields that could be default.
  If it's from normal update, it should be OK to keep original
  default values in the physical record as is, however,
  if it's from rollback, it may rollback an update from default
  value to non-default. To make the rolled back record as is,
  it has to check all possible default values. */
  for (; n_fields > index->get_instant_fields(); --n_fields) {
    const dict_col_t *col = index->get_field(n_fields - 1)->col;
    const dfield_t *dfield = dtuple_get_nth_field(this, n_fields - 1);
    ulint len = dfield_get_len(dfield);

    ut_ad(col->instant_default != nullptr);

    if (len != col->instant_default->len ||
        (len != UNIV_SQL_NULL &&
         memcmp(dfield_get_data(dfield), col->instant_default->value, len) !=
             0)) {
      break;
    }
  }
}

#endif /* !UNIV_HOTBACKUP */
