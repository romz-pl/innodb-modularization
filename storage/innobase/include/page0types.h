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

/** @file include/page0types.h
 Index page routines

 Created 2/2/1994 Heikki Tuuri
 *******************************************************/

#ifndef page0types_h
#define page0types_h

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>
#include <innodb/page/page_zip_stat_t.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_zip_rec_set_deleted.h>
#include <innodb/page/page_zip_rec_set_owned.h>
#include <innodb/page/page_zip_dir_add_slot.h>
#include <innodb/dict_types/index_id_t.h>


#include <map>





/** Index page cursor */
struct page_cur_t;



/** Compression statistics types */
typedef std::map<index_id_t, page_zip_stat_t, std::less<index_id_t>,
                 ut_allocator<std::pair<const index_id_t, page_zip_stat_t>>>
    page_zip_stat_per_index_t;


/** Statistics on compression, indexed by dict_index_t::id */
extern page_zip_stat_per_index_t page_zip_stat_per_index;





/** Shift the dense page directory when a record is deleted.
@param[in,out]	page_zip	compressed page
@param[in]	rec		deleted record
@param[in]	index		index of rec
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	free		previous start of the free list */
void page_zip_dir_delete(page_zip_des_t *page_zip, byte *rec,
                         dict_index_t *index, const ulint *offsets,
                         const byte *free);


#endif
