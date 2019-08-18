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
#include <innodb/page/FILE_LOGFILE.h>
#include <innodb/page/LOGFILE.h>
#include <innodb/page/deflate.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_zip_available.h>
#include <innodb/page/page_zip_clear_rec.h>
#include <innodb/page/page_zip_compress.h>
#include <innodb/page/page_zip_compress_clust.h>
#include <innodb/page/page_zip_compress_clust_ext.h>
#include <innodb/page/page_zip_compress_dbg.h>
#include <innodb/page/page_zip_compress_deflate.h>
#include <innodb/page/page_zip_compress_log.h>
#include <innodb/page/page_zip_compress_node_ptrs.h>
#include <innodb/page/page_zip_compress_sec.h>
#include <innodb/page/page_zip_compress_write_log.h>
#include <innodb/page/page_zip_compress_write_log_no_data.h>
#include <innodb/page/page_zip_copy_recs.h>
#include <innodb/page/page_zip_decompress.h>
#include <innodb/page/page_zip_decompress_low.h>
#include <innodb/page/page_zip_dir_add_slot.h>
#include <innodb/page/page_zip_dir_delete.h>
#include <innodb/page/page_zip_dir_find_free.h>
#include <innodb/page/page_zip_dir_find_low.h>
#include <innodb/page/page_zip_dir_insert.h>
#include <innodb/page/page_zip_dir_start_offs.h>
#include <innodb/page/page_zip_dir_user_size.h>
#include <innodb/page/page_zip_empty_size.h>
#include <innodb/page/page_zip_fields_encode.h>
#include <innodb/page/page_zip_get_n_prev_extern.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_trailer_len.h>
#include <innodb/page/page_zip_header_cmp.h>
#include <innodb/page/page_zip_header_cmp.h>
#include <innodb/page/page_zip_hexdump.h>
#include <innodb/page/page_zip_hexdump_func.h>
#include <innodb/page/page_zip_is_too_big.h>
#include <innodb/page/page_zip_level.h>
#include <innodb/page/page_zip_log_pages.h>
#include <innodb/page/page_zip_max_ins_size.h>
#include <innodb/page/page_zip_parse_compress.h>
#include <innodb/page/page_zip_parse_write_blob_ptr.h>
#include <innodb/page/page_zip_parse_write_header.h>
#include <innodb/page/page_zip_parse_write_node_ptr.h>
#include <innodb/page/page_zip_rec_needs_ext.h>
#include <innodb/page/page_zip_rec_needs_ext.h>
#include <innodb/page/page_zip_rec_set_owned.h>
#include <innodb/page/page_zip_reorganize.h>
#include <innodb/page/page_zip_reset_stat_per_index.h>
#include <innodb/page/page_zip_reset_stat_per_index.h>
#include <innodb/page/page_zip_set_alloc.h>
#include <innodb/page/page_zip_stat_per_index.h>
#include <innodb/page/page_zip_validate_header_only.h>
#include <innodb/page/page_zip_validate_low.h>
#include <innodb/page/page_zip_write_blob_ptr.h>
#include <innodb/page/page_zip_write_header.h>
#include <innodb/page/page_zip_write_header_log.h>
#include <innodb/page/page_zip_write_node_ptr.h>
#include <innodb/page/page_zip_write_rec.h>
#include <innodb/page/page_zip_write_rec_ext.h>
#include <innodb/page/page_zip_write_trx_id_and_roll_ptr.h>


#include "mtr0log.h"
#include "page0page.h"
#include "srv0srv.h"
#include <sys/types.h>
#include <zlib.h>
#include "buf0buf.h"



#endif /* page0zip_h */
