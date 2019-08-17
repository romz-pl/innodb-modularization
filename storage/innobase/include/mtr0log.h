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

/** @file include/mtr0log.h
 Mini-transaction logging routines

 Created 12/7/1995 Heikki Tuuri
 *******************************************************/

#ifndef mtr0log_h
#define mtr0log_h

#include <innodb/univ/univ.h>

// Forward declaration
struct dict_index_t;

#include <innodb/mtr/mtr_buf_t.h>
#include <innodb/log_types/mlog_id_t.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mlog_write_ulint.h>
#include <innodb/mtr/mlog_write_ull.h>
#include <innodb/mtr/mlog_log_string.h>
#include <innodb/mtr/mlog_write_string.h>
#include <innodb/mtr/mlog_catenate_ulint.h>
#include <innodb/mtr/mlog_catenate_ulint_compressed.h>
#include <innodb/mtr/mlog_write_initial_log_record.h>
#include <innodb/mtr/mlog_catenate_string.h>
#include <innodb/mtr/mlog_catenate_ull_compressed.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_open_metadata.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_write_initial_dict_log_record.h>
#include <innodb/mtr/mlog_write_initial_log_record_low.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mlog_parse_initial_dict_log_record.h>
#include <innodb/mtr/mlog_parse_initial_log_record.h>
#include <innodb/mtr/mlog_open_and_write_index.h>







/** Parses a log record written by mlog_write_ulint or mlog_write_ull.
 @return parsed record end, NULL if not a complete record */
byte *mlog_parse_nbytes(
    mlog_id_t type,      /*!< in: log record type: MLOG_1BYTE, ... */
    const byte *ptr,     /*!< in: buffer */
    const byte *end_ptr, /*!< in: buffer end */
    byte *page,          /*!< in: page where to apply the log record,
                         or NULL */
    void *page_zip);     /*!< in/out: compressed page, or NULL */
/** Parses a log record written by mlog_write_string.
 @return parsed record end, NULL if not a complete record */
byte *mlog_parse_string(
    byte *ptr,       /*!< in: buffer */
    byte *end_ptr,   /*!< in: buffer end */
    byte *page,      /*!< in: page where to apply the log record, or NULL */
    void *page_zip); /*!< in/out: compressed page, or NULL */









/** Parses a log record written by mlog_open_and_write_index.
 @return parsed record end, NULL if not a complete record */
byte *mlog_parse_index(byte *ptr,           /*!< in: buffer */
                       const byte *end_ptr, /*!< in: buffer end */
                       ibool comp, /*!< in: TRUE=compact record format */
                       dict_index_t **index); /*!< out, own: dummy index */




#endif /* mtr0log_h */
