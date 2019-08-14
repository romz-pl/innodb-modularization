/*****************************************************************************

Copyright (c) 2014, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/gis0type.h
 R-tree header file

 Created 2013/03/27 Jimmy Yang
 ***********************************************************************/

#ifndef gis0type_h
#define gis0type_h

#include <innodb/univ/univ.h>

#include <innodb/buf_block/buf_block_t.h>

#include "buf0buf.h"


#include "gis0geo.h"


#include "que0types.h"

#include "row0types.h"
#include "trx0types.h"
#include "ut0vec.h"
#include "ut0wqueue.h"

#include <list>
#include <vector>

struct dfield_t;
struct dtuple_t;


#include <innodb/gis_type/node_seq_t.h>
#include <innodb/gis_type/node_visit_t.h>
#include <innodb/gis_type/rtr_node_path_t.h>
#include <innodb/gis_type/rtr_rec_t.h>
#include <innodb/gis_type/rtr_rec_vector.h>
#include <innodb/gis_type/rtr_ssn_t.h>
#include <innodb/gis_type/flags.h>
#include <innodb/gis_type/rtr_info_t.h>
#include <innodb/gis_type/rtr_info_active.h>
#include <innodb/gis_type/rtr_info_track_t.h>
#include <innodb/gis_type/rtr_rec_move_t.h>


/* Structure for matched records on the leaf page */
typedef struct matched_rec {
  byte *bufp; /*!< aligned buffer point */
  byte rec_buf[UNIV_PAGE_SIZE_MAX * 2];
  /*!< buffer used to copy matching rec */
  buf_block_t block;            /*!< the shadow buffer block */
  ulint used;                   /*!< memory used */
  rtr_rec_vector *matched_recs; /*!< vector holding the matching rec */
  ib_mutex_t rtr_match_mutex;   /*!< mutex protect the match_recs
                                vector */
  bool valid;                   /*!< whether result in matched_recs
                                or this search is valid (page not
                                dropped) */
  bool locked;                  /*!< whether these recs locked */
} matched_rec_t;


#endif /*!< gis0rtree.h */
