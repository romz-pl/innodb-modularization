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

/** @file include/page0zip.h
 Compressed page interface

 Created June 2005 by Marko Makela
 *******************************************************/

#ifndef page0zip_h
#define page0zip_h

#include <innodb/univ/univ.h>

#include <innodb/crc32/crc32.h>
#include <innodb/disk/page_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_zip_available.h>
#include <innodb/page/page_zip_decompress_low.h>
#include <innodb/page/page_zip_dir_find_free.h>
#include <innodb/page/page_zip_dir_find_low.h>
#include <innodb/page/page_zip_dir_start_offs.h>
#include <innodb/page/page_zip_dir_user_size.h>
#include <innodb/page/page_zip_empty_size.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_trailer_len.h>
#include <innodb/page/page_zip_level.h>
#include <innodb/page/page_zip_log_pages.h>
#include <innodb/page/page_zip_max_ins_size.h>
#include <innodb/page/page_zip_rec_needs_ext.h>
#include <innodb/page/page_zip_rec_needs_ext.h>
#include <innodb/page/page_zip_reset_stat_per_index.h>
#include <innodb/page/page_zip_stat_per_index.h>
#include <innodb/page/page_zip_write_header.h>
#include <innodb/page/page_zip_write_header_log.h>
#include <innodb/page/page_zip_get_n_prev_extern.h>
#include <innodb/page/page_zip_compress_clust.h>

#include "mtr0log.h"
#include "page0page.h"
#include "page0zip.h"
#include "srv0srv.h"
#include <sys/types.h>
#include <zlib.h>
#include "buf0buf.h"


/** Shift the dense page directory when a record is deleted.
@param[in,out]	page_zip	compressed page
@param[in]	rec		deleted record
@param[in]	index		index of rec
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	free		previous start of the free list */
void page_zip_dir_delete(page_zip_des_t *page_zip, byte *rec,
                         dict_index_t *index, const ulint *offsets,
                         const byte *free);




#include <innodb/page/page_zip_compress_write_log.h>
#include <innodb/page/page_zip_is_too_big.h>
#include <innodb/page/page_zip_set_alloc.h>
#include <innodb/page/page_zip_fields_encode.h>
#include <innodb/page/page_zip_compress_node_ptrs.h>
#include <innodb/page/page_zip_compress_sec.h>
#include <innodb/page/page_zip_compress_clust_ext.h>
#include <innodb/page/page_zip_compress.h>
#include <innodb/page/page_zip_decompress.h>





#ifdef UNIV_ZIP_DEBUG
/** Check that the compressed and decompressed pages match.
 @return true if valid, false if not */
ibool page_zip_validate_low(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const page_t *page,             /*!< in: uncompressed page */
    const dict_index_t *index,      /*!< in: index of the page, if known */
    ibool sloppy);                  /*!< in: FALSE=strict,
                            TRUE=ignore the MIN_REC_FLAG */

#endif                              /* UNIV_ZIP_DEBUG */




/** Write an entire record on the compressed page.  The data must already
 have been written to the uncompressed page. */
void page_zip_write_rec(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in: record being written */
    const dict_index_t *index, /*!< in: the index the record belongs to */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint create);             /*!< in: nonzero=insert, zero=update */

/** Parses a log record of writing a BLOB pointer of a record.
 @return end of log record or NULL */
byte *page_zip_parse_write_blob_ptr(
    byte *ptr,                 /*!< in: redo log buffer */
    byte *end_ptr,             /*!< in: redo log buffer end */
    page_t *page,              /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip); /*!< in/out: compressed page */

/** Write a BLOB pointer of a record on the leaf page of a clustered index.
 The information must already have been updated on the uncompressed page. */
void page_zip_write_blob_ptr(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in/out: record whose data is being
                               written */
    const dict_index_t *index, /*!< in: index of the page */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint n,                   /*!< in: column index */
    mtr_t *mtr);               /*!< in: mini-transaction handle,
                       or NULL if no logging is needed */

/** Parses a log record of writing the node pointer of a record.
 @return end of log record or NULL */
byte *page_zip_parse_write_node_ptr(
    byte *ptr,                 /*!< in: redo log buffer */
    byte *end_ptr,             /*!< in: redo log buffer end */
    page_t *page,              /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip); /*!< in/out: compressed page */

/** Write the node pointer of a record on a non-leaf compressed page. */
void page_zip_write_node_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    ulint size,               /*!< in: data size of rec */
    ulint ptr,                /*!< in: node pointer */
    mtr_t *mtr);              /*!< in: mini-transaction, or NULL */

/** Write the trx_id and roll_ptr of a record on a B-tree leaf node page. */
void page_zip_write_trx_id_and_roll_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    const ulint *offsets,     /*!< in: rec_get_offsets(rec, index) */
    ulint trx_id_col,         /*!< in: column number of TRX_ID in rec */
    trx_id_t trx_id,          /*!< in: transaction identifier */
    roll_ptr_t roll_ptr);     /*!< in: roll_ptr */



/** Write the "owned" flag of a record on a compressed page.  The n_owned field
 must already have been written on the uncompressed page. */
void page_zip_rec_set_owned(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *rec,          /*!< in: record on the uncompressed page */
    ulint flag);              /*!< in: the owned flag (nonzero=TRUE) */

/** Insert a record to the dense page directory. */
void page_zip_dir_insert(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *prev_rec,     /*!< in: record after which to insert */
    const byte *free_rec,     /*!< in: record from which rec was
                             allocated, or NULL */
    byte *rec);               /*!< in: record to insert */

/** Shift the dense page directory and the array of BLOB pointers when a record
is deleted.
@param[in,out]	page_zip	compressed page
@param[in]	rec		deleted record
@param[in]	index		index of rec
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	free		previous start of the free list */
void page_zip_dir_delete(page_zip_des_t *page_zip, byte *rec,
                         const dict_index_t *index, const ulint *offsets,
                         const byte *free);

/** Add a slot to the dense page directory. */
void page_zip_dir_add_slot(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    bool is_clustered);       /*!< in: nonzero for clustered index,
                              zero for others */

/** Parses a log record of writing to the header of a page.
 @return end of log record or NULL */
byte *page_zip_parse_write_header(
    byte *ptr,                 /*!< in: redo log buffer */
    byte *end_ptr,             /*!< in: redo log buffer end */
    page_t *page,              /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip); /*!< in/out: compressed page */


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
    mtr_t *mtr);         /*!< in: mini-transaction */
/** Copy the records of a page byte for byte.  Do not copy the page header
 or trailer, except those B-tree header fields that are directly
 related to the storage of records.  Also copy PAGE_MAX_TRX_ID.
 NOTE: The caller must update the lock table and the adaptive hash index. */
void page_zip_copy_recs(
    page_zip_des_t *page_zip,      /*!< out: copy of src_zip
                                   (n_blobs, m_start, m_end,
                                   m_nonempty, data[0..size-1]) */
    page_t *page,                  /*!< out: copy of src */
    const page_zip_des_t *src_zip, /*!< in: compressed page */
    const page_t *src,             /*!< in: page */
    dict_index_t *index,           /*!< in: index of the B-tree */
    mtr_t *mtr);                   /*!< in: mini-transaction */
#ifndef UNIV_HOTBACKUP
#endif /* !UNIV_HOTBACKUP */

/** Parses a log record of compressing an index page.
 @return end of log record or NULL */
byte *page_zip_parse_compress(
    byte *ptr,                 /*!< in: buffer */
    byte *end_ptr,             /*!< in: buffer end */
    page_t *page,              /*!< out: uncompressed page */
    page_zip_des_t *page_zip); /*!< out: compressed page */

#include <innodb/page/page_zip_compress_write_log_no_data.h>



#ifndef UNIV_HOTBACKUP
/** Reset the counters used for filling
 INFORMATION_SCHEMA.innodb_cmp_per_index. */
UNIV_INLINE
void page_zip_reset_stat_per_index();

#ifdef UNIV_MATERIALIZE
#undef UNIV_INLINE
#define UNIV_INLINE UNIV_INLINE_ORIGINAL
#endif
#endif /* !UNIV_HOTBACKUP */


#endif /* page0zip_h */
