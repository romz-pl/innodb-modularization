/*****************************************************************************

Copyright (c) 2005, 2018, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2012, Facebook Inc.

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

/** @file page/page0zip.cc
 Compressed page interface

 Created June 2005 by Marko Makela
 *******************************************************/

#include <innodb/record/rec_get_n_extern_new.h>
#include <innodb/record/rec_get_converted_size_comp_prefix.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/assert/ASSERT_ZERO.h>
#include <innodb/assert/ASSERT_ZERO_BLOB.h>
#include <innodb/buf_frame/buf_frame_copy.h>
#include <innodb/disk/page_size_t.h>
#include <innodb/page/page_zip_dir_encode.h>
#include <innodb/page/page_zip_dir_find.h>
#include <innodb/page/page_zip_dir_start.h>
#include <innodb/page/page_zip_dir_start_low.h>
#include <innodb/page/page_zip_fixed_field_encode.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_extra_size.h>
#include <innodb/record/rec_offs_n_extern.h>
#include <innodb/time/ut_time_us.h>
#include <innodb/page/page_zip_dir_get.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/crc32/crc32.h>
#include <innodb/page/page_zip_stat.h>

#include "page0zip.h"
#include "btr0cur.h"
#include "dict0dict.h"
#include "log0recv.h"
#include "mtr0log.h"
#include "page0page.h"
#ifndef UNIV_HOTBACKUP
#include "btr0sea.h"
#include "buf0buf.h"
#include "buf0lru.h"
#include "dict0boot.h"
#include "lock0lock.h"
#include "srv0mon.h"
#include "srv0srv.h"
#endif /* !UNIV_HOTBACKUP */

#include <zlib.h>
#include <algorithm>
#include <map>

/** A BLOB field reference full of zero, for use in assertions and tests.
Initially, BLOB field references are set to zero, in
dtuple_convert_big_rec(). */
const byte field_ref_zero[FIELD_REF_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};







#ifdef UNIV_HOTBACKUP

#define lock_move_reorganize_page(block, temp_block) ((void)0)
#define buf_LRU_stat_inc_unzip() ((void)0)
#define MONITOR_INC(x) ((void)0)
#endif /* !UNIV_HOTBACKUP */



#if 0 || defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
/** Symbol for enabling compression and decompression diagnostics */
#define PAGE_ZIP_COMPRESS_DBG
#endif



/** Reorganize and compress a page.  This is a low-level operation for
 compressed pages, to be used when page_zip_compress() fails.
 On success, a redo log entry MLOG_ZIP_PAGE_COMPRESS will be written.
 The function btr_page_reorganize() should be preferred whenever possible.
 IMPORTANT: if page_zip_reorganize() is invoked on a leaf page of a
 non-clustered index, the caller must update the insert buffer free
 bits in the same mini-transaction in such a way that the modification
 will be redo-logged.
 @return true on success, false on failure; page_zip will be left
 intact on failure, but page will be overwritten. */
ibool page_zip_reorganize(
    buf_block_t *block,  /*!< in/out: page with compressed page;
                         on the compressed page, in: size;
                         out: data, n_blobs,
                         m_start, m_end, m_nonempty */
    dict_index_t *index, /*!< in: index of the B-tree node */
    mtr_t *mtr)          /*!< in: mini-transaction */
{
#ifndef UNIV_HOTBACKUP
  buf_pool_t *buf_pool = buf_pool_from_block(block);
#endif /* !UNIV_HOTBACKUP */
  page_zip_des_t *page_zip = buf_block_get_page_zip(block);
  page_t *page = buf_block_get_frame(block);
  buf_block_t *temp_block;
  page_t *temp_page;

#ifndef UNIV_HOTBACKUP
  ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
#endif /* !UNIV_HOTBACKUP */
  ut_ad(page_is_comp(page));
  ut_ad(!dict_index_is_ibuf(index));
  ut_ad(!index->table->is_temporary());
  /* Note that page_zip_validate(page_zip, page, index) may fail here. */
  UNIV_MEM_ASSERT_RW(page, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  /* Disable logging */
  mtr_log_t log_mode = mtr_set_log_mode(mtr, MTR_LOG_NONE);

#ifndef UNIV_HOTBACKUP
  temp_block = buf_block_alloc(buf_pool);
  btr_search_drop_page_hash_index(block);
#else  /* !UNIV_HOTBACKUP */
  temp_block = back_block2;
#endif /* !UNIV_HOTBACKUP */
  temp_page = temp_block->frame;

  /* Copy the old page to temporary space */
  buf_frame_copy(temp_page, page);

  /* Recreate the page: note that global data on page (possible
  segment headers, next page-field, etc.) is preserved intact */

  page_create(block, mtr, TRUE, fil_page_get_type(page));

  /* Copy the records from the temporary space to the recreated page;
  do not copy the lock bits yet */

  page_copy_rec_list_end_no_locks(block, temp_block,
                                  page_get_infimum_rec(temp_page), index, mtr);

  /* Temp-Tables are not shared across connection and so we avoid
  locking of temp-tables as there would be no 2 trx trying to
  operate on same temp-table in parallel.
  max_trx_id is use to track which all trxs wrote to the page
  in parallel but in case of temp-table this can is not needed. */
  if (!index->is_clustered() && page_is_leaf(temp_page)) {
    /* Copy max trx id to recreated page */
    trx_id_t max_trx_id = page_get_max_trx_id(temp_page);
    page_set_max_trx_id(block, NULL, max_trx_id, NULL);
    ut_ad(max_trx_id != 0);
  }

  /* Restore logging. */
  mtr_set_log_mode(mtr, log_mode);

  if (!page_zip_compress(page_zip, page, index, page_zip_level, mtr)) {
#ifndef UNIV_HOTBACKUP
    buf_block_free(temp_block);
#endif /* !UNIV_HOTBACKUP */
    return (FALSE);
  }

  lock_move_reorganize_page(block, temp_block);

#ifndef UNIV_HOTBACKUP
  buf_block_free(temp_block);
#endif /* !UNIV_HOTBACKUP */
  return (TRUE);
}

