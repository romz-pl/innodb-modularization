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

#include <stddef.h>
#include <sys/types.h>

/** @file include/page0page.h
 Index page routines

 Created 2/2/1994 Heikki Tuuri
 *******************************************************/

#ifndef page0page_h
#define page0page_h

#include <innodb/univ/univ.h>

#include <innodb/page/page_size_validate.h>
#include <innodb/page/page_get_max_insert_size_after_reorganize.h>
#include <innodb/page/page_get_max_insert_size.h>
#include <innodb/page/page_get_data_size.h>
#include <innodb/page/page_rec_get_base_extra_size.h>
#include <innodb/page/page_get_page_no.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_dir_get_n_slots.h>
#include <innodb/page/page_dir_calc_reserved_space.h>
#include <innodb/page/page_dir_slot_get_n_owned.h>
#include <innodb/page/page_dir_slot_set_rec.h>
#include <innodb/page/page_dir_slot_get_rec.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_get_supremum_rec.h>
#include <innodb/page/page_get_infimum_rec.h>
#include <innodb/page/page_rec_is_infimum.h>
#include <innodb/page/page_rec_is_supremum.h>
#include <innodb/page/page_rec_is_user_rec.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_is_infimum_low.h>
#include <innodb/page/page_rec_is_supremum_low.h>
#include <innodb/page/page_dir_slot_t.h>
#include <innodb/page/page_dir_t.h>
#include <innodb/page/flag.h>
#include <innodb/page/infimum_extra.h>
#include <innodb/page/infimum_data.h>
#include <innodb/page/supremum_extra_data.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_get_free_space_of_empty.h>
#include <innodb/page/page_header_get_offs.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_rec_is_comp.h>
#include <innodb/page/page_rec_get_heap_no.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_is_empty.h>
#include <innodb/page/page_is_root.h>
#include <innodb/page/page_has_garbage.h>
#include <innodb/page/page_get_infimum_offset.h>
#include <innodb/page/page_get_supremum_offset.h>
#include <innodb/page/page_rec_is_user_rec_low.h>
#include <innodb/page/page_dir_find_owner_slot.h>
#include <innodb/page/page_dir_slot_check.h>

#include "buf0buf.h"
#include "data0data.h"
#include "dict0dict.h"
#include "fil0fil.h"
#include "fsp0fsp.h"

#include "rem0rec.h"

#ifdef UNIV_MATERIALIZE
#undef UNIV_INLINE
#define UNIV_INLINE
#endif








/** Returns the max trx id field value. */
UNIV_INLINE
trx_id_t page_get_max_trx_id(const page_t *page); /*!< in: page */
/** Sets the max trx id field value. */
void page_set_max_trx_id(
    buf_block_t *block,       /*!< in/out: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    trx_id_t trx_id,          /*!< in: transaction id */
    mtr_t *mtr);              /*!< in/out: mini-transaction, or NULL */

/** Sets the max trx id field value if trx_id is bigger than the previous
value.
@param[in,out]	block		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	trx_id		transaction id
@param[in,out]	mtr		mini-transaction */
UNIV_INLINE
void page_update_max_trx_id(buf_block_t *block, page_zip_des_t *page_zip,
                            trx_id_t trx_id, mtr_t *mtr);

/** Returns the RTREE SPLIT SEQUENCE NUMBER (FIL_RTREE_SPLIT_SEQ_NUM).
@param[in]	page	page
@return SPLIT SEQUENCE NUMBER */
UNIV_INLINE
node_seq_t page_get_ssn_id(const page_t *page);

/** Sets the RTREE SPLIT SEQUENCE NUMBER field value
@param[in,out]	block		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	ssn_id		split sequence id
@param[in,out]	mtr		mini-transaction */
UNIV_INLINE
void page_set_ssn_id(buf_block_t *block, page_zip_des_t *page_zip,
                     node_seq_t ssn_id, mtr_t *mtr);


/** Sets the given header field.
@param[in,out]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	field		PAGE_N_DIR_SLOTS, ...
@param[in]	val		value */
UNIV_INLINE
void page_header_set_field(page_t *page, page_zip_des_t *page_zip, ulint field,
                           ulint val);



/** Returns the pointer stored in the given header field, or NULL. */
#define page_header_get_ptr(page, field)          \
  (page_header_get_offs(page, field)              \
       ? page + page_header_get_offs(page, field) \
       : NULL)

/** Sets the pointer stored in the given header field.
@param[in,out]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in,out]	field		PAGE_FREE, ...
@param[in]	ptr		pointer or NULL */
UNIV_INLINE
void page_header_set_ptr(page_t *page, page_zip_des_t *page_zip, ulint field,
                         const byte *ptr);
#ifndef UNIV_HOTBACKUP

/** Resets the last insert info field in the page header. Writes to mlog about
this operation.
@param[in]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	mtr		mtr */
UNIV_INLINE
void page_header_reset_last_insert(page_t *page, page_zip_des_t *page_zip,
                                   mtr_t *mtr);
#endif /* !UNIV_HOTBACKUP */






/** Returns the nth record of the record list.
 This is the inverse function of page_rec_get_n_recs_before().
 @return nth record */
const rec_t *page_rec_get_nth_const(const page_t *page, /*!< in: page */
                                    ulint nth)          /*!< in: nth record */
    MY_ATTRIBUTE((warn_unused_result));

/** Returns the nth record of the record list.
This is the inverse function of page_rec_get_n_recs_before().
@param[in]	page	page
@param[in]	nth	nth record
@return nth record */
UNIV_INLINE
rec_t *page_rec_get_nth(page_t *page, ulint nth)
    MY_ATTRIBUTE((warn_unused_result));

#ifndef UNIV_HOTBACKUP
/** Returns the middle record of the records on the page. If there is an
 even number of records in the list, returns the first record of the
 upper half-list.
 @return middle record */
UNIV_INLINE
rec_t *page_get_middle_rec(page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* !UNIV_HOTBACKUP */

/** Gets the tablespace identifier.
 @return space id */
UNIV_INLINE
space_id_t page_get_space_id(const page_t *page); /*!< in: page */

/** Returns the number of records before the given record in chain.
 The number includes infimum and supremum records.
 This is the inverse function of page_rec_get_nth().
 @return number of records */
ulint page_rec_get_n_recs_before(
    const rec_t *rec); /*!< in: the physical record */


/** Sets the number of records in the heap.
@param[in,out]	page		index page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL. Note that the size of the
                                dense page directory in the compressed page
                                trailer is n_heap * PAGE_ZIP_DIR_SLOT_SIZE.
@param[in]	n_heap		number of records*/
UNIV_INLINE
void page_dir_set_n_heap(page_t *page, page_zip_des_t *page_zip, ulint n_heap);



/** Sets the number of dir slots in directory.
@param[in,out]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	n_slots		number of slots */
UNIV_INLINE
void page_dir_set_n_slots(page_t *page, page_zip_des_t *page_zip,
                          ulint n_slots);









/** This is used to set the owned records field of a directory slot.
@param[in,out]	slot		directory slot
@param[in,out]	page_zip	compressed page, or NULL
@param[in]	n		number of records owned by the slot */
UNIV_INLINE
void page_dir_slot_set_n_owned(page_dir_slot_t *slot, page_zip_des_t *page_zip,
                               ulint n);






/** Gets the pointer to the next record on the page.
@param[in]	rec	pointer to record
@param[in]	comp	nonzero=compact page layout
@return pointer to next record */
UNIV_INLINE
const rec_t *page_rec_get_next_low(const rec_t *rec, ulint comp);

/** Gets the pointer to the next record on the page.
 @return pointer to next record */
UNIV_INLINE
rec_t *page_rec_get_next(rec_t *rec); /*!< in: pointer to record */
/** Gets the pointer to the next record on the page.
 @return pointer to next record */
UNIV_INLINE
const rec_t *page_rec_get_next_const(
    const rec_t *rec); /*!< in: pointer to record */
/** Gets the pointer to the next non delete-marked record on the page.
 If all subsequent records are delete-marked, then this function
 will return the supremum record.
 @return pointer to next non delete-marked record or pointer to supremum */
UNIV_INLINE
const rec_t *page_rec_get_next_non_del_marked(
    const rec_t *rec); /*!< in: pointer to record */

/** Sets the pointer to the next record on the page.
@param[in]	rec	pointer to record, must not be page supremum
@param[in]	next	pointer to next record, must not be page infimum */
UNIV_INLINE
void page_rec_set_next(rec_t *rec, const rec_t *next);

/** Gets the pointer to the previous record.
 @return pointer to previous record */
UNIV_INLINE
const rec_t *page_rec_get_prev_const(
    const rec_t *rec); /*!< in: pointer to record, must not be page
                       infimum */
/** Gets the pointer to the previous record.
 @return pointer to previous record */
UNIV_INLINE
rec_t *page_rec_get_prev(rec_t *rec); /*!< in: pointer to record,
                                      must not be page infimum */








/** true if the record is the first user record on a page.
 @return true if the first user record */
UNIV_INLINE
bool page_rec_is_first(const rec_t *rec,   /*!< in: record */
                       const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));

/** true if the record is the second user record on a page.
 @return true if the second user record */
UNIV_INLINE
bool page_rec_is_second(const rec_t *rec,   /*!< in: record */
                        const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));

/** true if the record is the last user record on a page.
 @return true if the last user record */
UNIV_INLINE
bool page_rec_is_last(const rec_t *rec,   /*!< in: record */
                      const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));

/** true if the record is the second last user record on a page.
 @return true if the second last user record */
UNIV_INLINE
bool page_rec_is_second_last(const rec_t *rec,   /*!< in: record */
                             const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));

/** Looks for the record which owns the given record.
 @return the owner record */
UNIV_INLINE
rec_t *page_rec_find_owner_rec(rec_t *rec); /*!< in: the physical record */
#ifndef UNIV_HOTBACKUP

/** Write a 32-bit field in a data dictionary record.
@param[in,out]	rec	record to update
@param[in]	i	index of the field to update
@param[in]	val	value to write
@param[in,out]	mtr	mini-transaction */
UNIV_INLINE
void page_rec_write_field(rec_t *rec, ulint i, ulint val, mtr_t *mtr);
#endif /* !UNIV_HOTBACKUP */






/** Allocates a block of memory from the head of the free list of an index
page.
@param[in,out]	page		index page
@param[in,out]	page_zip	compressed page with enough space available
                                for inserting the record, or NULL
@param[in]	next_rec	pointer to the new head of the free record
                                list
@param[in]	need		number of bytes allocated */
UNIV_INLINE
void page_mem_alloc_free(page_t *page, page_zip_des_t *page_zip,
                         rec_t *next_rec, ulint need);

/** Allocates a block of memory from the heap of an index page.
 @return pointer to start of allocated buffer, or NULL if allocation fails */
byte *page_mem_alloc_heap(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page with enough
                             space available for inserting the record,
                             or NULL */
    ulint need,               /*!< in: total number of bytes needed */
    ulint *heap_no);          /*!< out: this contains the heap number
                             of the allocated record
                             if allocation succeeds */

/** Puts a record to free list.
@param[in,out]	page		index page
@param[in,out]	page_zip	compressed page, or NULL
@param[in]	rec		pointer to the (origin of) record
@param[in]	index		index of rec
@param[in]	offsets		array returned by rec_get_offsets() */
UNIV_INLINE
void page_mem_free(page_t *page, page_zip_des_t *page_zip, rec_t *rec,
                   const dict_index_t *index, const ulint *offsets);

/** Create an uncompressed B-tree or R-tree or SDI index page.
@param[in]	block		a buffer block where the page is created
@param[in]	mtr		mini-transaction handle
@param[in]	comp		nonzero=compact page format
@param[in]	page_type	page type
@return pointer to the page */
page_t *page_create(buf_block_t *block, mtr_t *mtr, ulint comp,
                    page_type_t page_type);

/** Create a compressed B-tree index page.
@param[in,out]	block		buffer frame where the page is created
@param[in]	index		index of the page, or NULL when applying
                                TRUNCATE log record during recovery
@param[in]	level		the B-tree level of the page
@param[in]	max_trx_id	PAGE_MAX_TRX_ID
@param[in]	mtr		mini-transaction handle
@param[in]	page_type	page_type to be created. Only FIL_PAGE_INDEX,
                                FIL_PAGE_RTREE, FIL_PAGE_SDI allowed */
page_t *page_create_zip(buf_block_t *block, dict_index_t *index, ulint level,
                        trx_id_t max_trx_id, mtr_t *mtr, page_type_t page_type);

/** Empty a previously created B-tree index page. */
void page_create_empty(buf_block_t *block,  /*!< in/out: B-tree block */
                       dict_index_t *index, /*!< in: the index of the page */
                       mtr_t *mtr);         /*!< in/out: mini-transaction */
/** Differs from page_copy_rec_list_end, because this function does not
 touch the lock table and max trx id on page or compress the page.

 IMPORTANT: The caller will have to update IBUF_BITMAP_FREE
 if new_block is a compressed leaf page in a secondary index.
 This has to be done either within the same mini-transaction,
 or by invoking ibuf_reset_free_bits() before mtr_commit(). */
void page_copy_rec_list_end_no_locks(
    buf_block_t *new_block, /*!< in: index page to copy to */
    buf_block_t *block,     /*!< in: index page of rec */
    rec_t *rec,             /*!< in: record on page */
    dict_index_t *index,    /*!< in: record descriptor */
    mtr_t *mtr);            /*!< in: mtr */
/** Copies records from page to new_page, from the given record onward,
 including that record. Infimum and supremum records are not copied.
 The records are copied to the start of the record list on new_page.

 IMPORTANT: The caller will have to update IBUF_BITMAP_FREE
 if new_block is a compressed leaf page in a secondary index.
 This has to be done either within the same mini-transaction,
 or by invoking ibuf_reset_free_bits() before mtr_commit().

 @return pointer to the original successor of the infimum record on
 new_page, or NULL on zip overflow (new_block will be decompressed) */
rec_t *page_copy_rec_list_end(
    buf_block_t *new_block, /*!< in/out: index page to copy to */
    buf_block_t *block,     /*!< in: index page containing rec */
    rec_t *rec,             /*!< in: record on page */
    dict_index_t *index,    /*!< in: record descriptor */
    mtr_t *mtr);            /*!< in: mtr */
/** Copies records from page to new_page, up to the given record, NOT
 including that record. Infimum and supremum records are not copied.
 The records are copied to the end of the record list on new_page.

 IMPORTANT: The caller will have to update IBUF_BITMAP_FREE
 if new_block is a compressed leaf page in a secondary index.
 This has to be done either within the same mini-transaction,
 or by invoking ibuf_reset_free_bits() before mtr_commit().

 @return pointer to the original predecessor of the supremum record on
 new_page, or NULL on zip overflow (new_block will be decompressed) */
rec_t *page_copy_rec_list_start(
    buf_block_t *new_block, /*!< in/out: index page to copy to */
    buf_block_t *block,     /*!< in: index page containing rec */
    rec_t *rec,             /*!< in: record on page */
    dict_index_t *index,    /*!< in: record descriptor */
    mtr_t *mtr);            /*!< in: mtr */
/** Deletes records from a page from a given record onward, including that
 record. The infimum and supremum records are not deleted. */
void page_delete_rec_list_end(
    rec_t *rec,          /*!< in: pointer to record on page */
    buf_block_t *block,  /*!< in: buffer block of the page */
    dict_index_t *index, /*!< in: record descriptor */
    ulint n_recs,        /*!< in: number of records to delete,
                         or ULINT_UNDEFINED if not known */
    ulint size,          /*!< in: the sum of the sizes of the
                         records in the end of the chain to
                         delete, or ULINT_UNDEFINED if not known */
    mtr_t *mtr);         /*!< in: mtr */
/** Deletes records from page, up to the given record, NOT including
 that record. Infimum and supremum records are not deleted. */
void page_delete_rec_list_start(
    rec_t *rec,          /*!< in: record on page */
    buf_block_t *block,  /*!< in: buffer block of the page */
    dict_index_t *index, /*!< in: record descriptor */
    mtr_t *mtr);         /*!< in: mtr */
/** Moves record list end to another page. Moved records include
 split_rec.

 IMPORTANT: The caller will have to update IBUF_BITMAP_FREE
 if new_block is a compressed leaf page in a secondary index.
 This has to be done either within the same mini-transaction,
 or by invoking ibuf_reset_free_bits() before mtr_commit().

 @return true on success; false on compression failure (new_block will
 be decompressed) */
ibool page_move_rec_list_end(
    buf_block_t *new_block, /*!< in/out: index page where to move */
    buf_block_t *block,     /*!< in: index page from where to move */
    rec_t *split_rec,       /*!< in: first record to move */
    dict_index_t *index,    /*!< in: record descriptor */
    mtr_t *mtr);            /*!< in: mtr */
/** Moves record list start to another page. Moved records do not include
 split_rec.

 IMPORTANT: The caller will have to update IBUF_BITMAP_FREE
 if new_block is a compressed leaf page in a secondary index.
 This has to be done either within the same mini-transaction,
 or by invoking ibuf_reset_free_bits() before mtr_commit().

 @return true on success; false on compression failure */
ibool page_move_rec_list_start(
    buf_block_t *new_block, /*!< in/out: index page where to move */
    buf_block_t *block,     /*!< in/out: page containing split_rec */
    rec_t *split_rec,       /*!< in: first record not to move */
    dict_index_t *index,    /*!< in: record descriptor */
    mtr_t *mtr);            /*!< in: mtr */
/** Splits a directory slot which owns too many records. */
void page_dir_split_slot(
    page_t *page,             /*!< in: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be written, or NULL */
    ulint slot_no);           /*!< in: the directory slot */
/** Tries to balance the given directory slot with too few records
 with the upper neighbor, so that there are at least the minimum number
 of records owned by the slot; this may result in the merging of
 two slots. */
void page_dir_balance_slot(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    ulint slot_no);           /*!< in: the directory slot */
/** Parses a log record of a record list end or start deletion.
 @return end of log record or NULL */
byte *page_parse_delete_rec_list(
    mlog_id_t type,      /*!< in: MLOG_LIST_END_DELETE,
                         MLOG_LIST_START_DELETE,
                         MLOG_COMP_LIST_END_DELETE or
                         MLOG_COMP_LIST_START_DELETE */
    byte *ptr,           /*!< in: buffer */
    byte *end_ptr,       /*!< in: buffer end */
    buf_block_t *block,  /*!< in/out: buffer block or NULL */
    dict_index_t *index, /*!< in: record descriptor */
    mtr_t *mtr);         /*!< in: mtr or NULL */

/** Parses a redo log record of creating a page.
@param[in,out]	block		buffer block, or NULL
@param[in]	comp		nonzero=compact page format
@param[in]	page_type	page type */
void page_parse_create(buf_block_t *block, ulint comp, page_type_t page_type);
#ifndef UNIV_HOTBACKUP
/** Prints record contents including the data relevant only in
 the index page context. */
void page_rec_print(const rec_t *rec,      /*!< in: physical record */
                    const ulint *offsets); /*!< in: record descriptor */
#ifdef UNIV_BTR_PRINT
/** This is used to print the contents of the directory for
 debugging purposes. */
void page_dir_print(page_t *page, /*!< in: index page */
                    ulint pr_n);  /*!< in: print n first and n last entries */
/** This is used to print the contents of the page record list for
 debugging purposes. */
void page_print_list(
    buf_block_t *block,  /*!< in: index page */
    dict_index_t *index, /*!< in: dictionary index of the page */
    ulint pr_n);         /*!< in: print n first and n last entries */
/** Prints the info in a page header. */
void page_header_print(const page_t *page); /*!< in: index page */
/** This is used to print the contents of the page for
 debugging purposes. */
void page_print(buf_block_t *block,  /*!< in: index page */
                dict_index_t *index, /*!< in: dictionary index of the page */
                ulint dn,            /*!< in: print dn first and last entries
                                     in directory */
                ulint rn);           /*!< in: print rn first and last records
                                     in directory */
#endif                               /* UNIV_BTR_PRINT */
#endif                               /* !UNIV_HOTBACKUP */
/** The following is used to validate a record on a page. This function
 differs from rec_validate as it can also check the n_owned field and
 the heap_no field.
 @return true if ok */
ibool page_rec_validate(
    const rec_t *rec,      /*!< in: physical record */
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */
#ifdef UNIV_DEBUG
/** Checks that the first directory slot points to the infimum record and
 the last to the supremum. This function is intended to track if the
 bug fixed in 4.0.14 has caused corruption to users' databases. */
void page_check_dir(const page_t *page); /*!< in: index page */
#endif                                   /* UNIV_DEBUG */
/** This function checks the consistency of an index page when we do not
 know the index. This is also resilient so that this should never crash
 even if the page is total garbage.
 @return true if ok */
ibool page_simple_validate_old(
    const page_t *page); /*!< in: index page in ROW_FORMAT=REDUNDANT */
/** This function checks the consistency of an index page when we do not
 know the index. This is also resilient so that this should never crash
 even if the page is total garbage.
 @return true if ok */
ibool page_simple_validate_new(
    const page_t *page); /*!< in: index page in ROW_FORMAT!=REDUNDANT */
/** This function checks the consistency of an index page.
 @return true if ok */
ibool page_validate(
    const page_t *page,   /*!< in: index page */
    dict_index_t *index); /*!< in: data dictionary index containing
                          the page record type definition */
/** Looks in the page record list for a record with the given heap number.
 @return record, NULL if not found */
const rec_t *page_find_rec_with_heap_no(
    const page_t *page, /*!< in: index page */
    ulint heap_no);     /*!< in: heap number */
/** Get the last non-delete-marked record on a page.
@param[in]	page	index tree leaf page
@return the last record, not delete-marked
@retval infimum record if all records are delete-marked */
const rec_t *page_find_rec_last_not_deleted(const page_t *page);

/** Issue a warning when the checksum that is stored in the page is valid,
but different than the global setting innodb_checksum_algorithm.
@param[in]	curr_algo	current checksum algorithm
@param[in]	page_checksum	page valid checksum
@param[in]	page_id		page identifier */
void page_warn_strict_checksum(srv_checksum_algorithm_t curr_algo,
                               srv_checksum_algorithm_t page_checksum,
                               const page_id_t &page_id);



#ifdef UNIV_MATERIALIZE
#undef UNIV_INLINE
#define UNIV_INLINE UNIV_INLINE_ORIGINAL
#endif

#include "page0page.ic"

#endif
