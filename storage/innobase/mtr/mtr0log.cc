/*****************************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file mtr/mtr0log.cc
 Mini-transaction log routines

 Created 12/7/1995 Heikki Tuuri
 *******************************************************/
#include <innodb/dict_mem/dict_mem_table_create.h>
#include <innodb/dict_mem/dict_mem_index_create.h>
#include <innodb/dict_mem/dict_mem_table_add_col.h>

#include "mtr0log.h"

#ifndef UNIV_HOTBACKUP
#include "buf0buf.h"
#include "buf0dblwr.h"
#include "dict0dict.h"
#include "log0recv.h"
#endif /* !UNIV_HOTBACKUP */
#include "page0page.h"

#ifndef UNIV_HOTBACKUP
#include "dict0boot.h"
#endif /* !UNIV_HOTBACKUP */









/** Parses a log record written by mlog_write_ulint or mlog_write_ull.
 @return parsed record end, NULL if not a complete record or a corrupt record */
byte *mlog_parse_nbytes(
    mlog_id_t type,      /*!< in: log record type: MLOG_1BYTE, ... */
    const byte *ptr,     /*!< in: buffer */
    const byte *end_ptr, /*!< in: buffer end */
    byte *page,          /*!< in: page where to apply the log
                         record, or NULL */
    void *page_zip)      /*!< in/out: compressed page, or NULL */
{
  ulint offset;
  ulint val;
  ib_uint64_t dval;

  ut_a(type <= MLOG_8BYTES);
  ut_a(!page || !page_zip || !fil_page_index_page_check(page));

  if (end_ptr < ptr + 2) {
    return (NULL);
  }

  offset = mach_read_from_2(ptr);
  ptr += 2;

  if (offset >= UNIV_PAGE_SIZE) {
    recv_sys->found_corrupt_log = TRUE;

    return (NULL);
  }

  if (type == MLOG_8BYTES) {
    dval = mach_u64_parse_compressed(&ptr, end_ptr);

    if (ptr == NULL) {
      return (NULL);
    }

    if (page) {
      if (page_zip) {
        mach_write_to_8(((page_zip_des_t *)page_zip)->data + offset, dval);
      }
      mach_write_to_8(page + offset, dval);
    }

    return (const_cast<byte *>(ptr));
  }

  val = mach_parse_compressed(&ptr, end_ptr);

  if (ptr == NULL) {
    return (NULL);
  }

  switch (type) {
    case MLOG_1BYTE:
      if (val > 0xFFUL) {
        goto corrupt;
      }
      if (page) {
        if (page_zip) {
          mach_write_to_1(((page_zip_des_t *)page_zip)->data + offset, val);
        }
        mach_write_to_1(page + offset, val);
      }
      break;
    case MLOG_2BYTES:
      if (val > 0xFFFFUL) {
        goto corrupt;
      }
      if (page) {
        if (page_zip) {
          mach_write_to_2(((page_zip_des_t *)page_zip)->data + offset, val);
        }
        mach_write_to_2(page + offset, val);
      }
      break;
    case MLOG_4BYTES:
      if (page) {
        if (page_zip) {
          mach_write_to_4(((page_zip_des_t *)page_zip)->data + offset, val);
        }
        mach_write_to_4(page + offset, val);
      }
      break;
    default:
    corrupt:
      recv_sys->found_corrupt_log = TRUE;
      ptr = NULL;
  }

  return (const_cast<byte *>(ptr));
}





/** Parses a log record written by mlog_write_string.
 @return parsed record end, NULL if not a complete record */
byte *mlog_parse_string(
    byte *ptr,      /*!< in: buffer */
    byte *end_ptr,  /*!< in: buffer end */
    byte *page,     /*!< in: page where to apply the log record, or NULL */
    void *page_zip) /*!< in/out: compressed page, or NULL */
{
  ulint offset;
  ulint len;

  ut_a(!page || !page_zip ||
       (fil_page_get_type(page) != FIL_PAGE_INDEX &&
        fil_page_get_type(page) != FIL_PAGE_RTREE));

  if (end_ptr < ptr + 4) {
    return (NULL);
  }

  offset = mach_read_from_2(ptr);
  ptr += 2;
  len = mach_read_from_2(ptr);
  ptr += 2;

  if (offset >= UNIV_PAGE_SIZE || len + offset > UNIV_PAGE_SIZE) {
    recv_sys->found_corrupt_log = TRUE;

    return (NULL);
  }

  if (end_ptr < ptr + len) {
    return (NULL);
  }

  if (page) {
    if (page_zip) {
      memcpy(((page_zip_des_t *)page_zip)->data + offset, ptr, len);
    }
    memcpy(page + offset, ptr, len);
  }

  return (ptr + len);
}


/** Parses a log record written by mlog_open_and_write_index.
 @return parsed record end, NULL if not a complete record */
byte *mlog_parse_index(byte *ptr,            /*!< in: buffer */
                       const byte *end_ptr,  /*!< in: buffer end */
                       ibool comp,           /*!< in: TRUE=compact row format */
                       dict_index_t **index) /*!< out, own: dummy index */
{
  ulint i;
  dict_table_t *table;
  dict_index_t *ind;
  bool instant = false;
  uint16_t n, n_uniq;
  uint16_t instant_cols = 0;

  ut_ad(comp == FALSE || comp == TRUE);

  if (comp) {
    if (end_ptr < ptr + 4) {
      return (NULL);
    }
    n = mach_read_from_2(ptr);
    ptr += 2;
    if ((n & 0x8000) != 0) {
      /* This is instant fields,
      see also mlog_open_and_write_index() */
      instant = true;
      instant_cols = n & ~0x8000;
      n = mach_read_from_2(ptr);
      ptr += 2;
      ut_ad((n & 0x8000) == 0);
      ut_ad(instant_cols <= n);

      if (end_ptr < ptr + 2) {
        return (nullptr);
      }
    }
    n_uniq = mach_read_from_2(ptr);
    ptr += 2;
    ut_ad(n_uniq <= n);
    if (end_ptr < ptr + n * 2) {
      return (NULL);
    }
  } else {
    n = n_uniq = 1;
  }
  table = dict_mem_table_create("LOG_DUMMY", DICT_HDR_SPACE, n, 0,
                                comp ? DICT_TF_COMPACT : 0, 0);
  if (instant) {
    table->set_instant_cols(instant_cols);
  }

  ind = dict_mem_index_create("LOG_DUMMY", "LOG_DUMMY", DICT_HDR_SPACE, 0, n);
  ind->table = table;
  ind->n_uniq = (unsigned int)n_uniq;
  if (n_uniq != n) {
    ut_a(n_uniq + DATA_ROLL_PTR <= n);
    ind->type = DICT_CLUSTERED;
  }
  if (comp) {
    for (i = 0; i < n; i++) {
      ulint len = mach_read_from_2(ptr);
      ptr += 2;
      /* The high-order bit of len is the NOT NULL flag;
      the rest is 0 or 0x7fff for variable-length fields,
      and 1..0x7ffe for fixed-length fields. */
      dict_mem_table_add_col(
          table, NULL, NULL,
          ((len + 1) & 0x7fff) <= 1 ? DATA_BINARY : DATA_FIXBINARY,
          len & 0x8000 ? DATA_NOT_NULL : 0, len & 0x7fff);

      /* The is_ascending flag does not matter during
      redo log apply, because we do not compare for
      "less than" or "greater than". */
      dict_index_add_col(ind, table, table->get_col(i), 0, true);
    }
    dict_table_add_system_columns(table, table->heap);
    if (n_uniq != n) {
      /* Identify DB_TRX_ID and DB_ROLL_PTR in the index. */
      ut_a(DATA_TRX_ID_LEN == ind->get_col(DATA_TRX_ID - 1 + n_uniq)->len);
      ut_a(DATA_ROLL_PTR_LEN == ind->get_col(DATA_ROLL_PTR - 1 + n_uniq)->len);
      ind->fields[DATA_TRX_ID - 1 + n_uniq].col = &table->cols[n + DATA_TRX_ID];
      ind->fields[DATA_ROLL_PTR - 1 + n_uniq].col =
          &table->cols[n + DATA_ROLL_PTR];
    }

    if (ind->is_clustered() && ind->table->has_instant_cols()) {
      ind->instant_cols = true;
      ind->n_instant_nullable =
          ind->get_n_nullable_before(ind->get_instant_fields());
    } else {
      ind->instant_cols = false;
      ind->n_instant_nullable = ind->n_nullable;
    }
  }
  /* avoid ut_ad(index->cached) in dict_index_get_n_unique_in_tree */
  ind->cached = TRUE;
  *index = ind;
  return (ptr);
}
