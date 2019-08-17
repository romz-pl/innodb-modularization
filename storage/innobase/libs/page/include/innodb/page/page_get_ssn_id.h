#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/gis_type/node_seq_t.h>

/** Returns the RTREE SPLIT SEQUENCE NUMBER (FIL_RTREE_SPLIT_SEQ_NUM).
@param[in]	page	page
@return SPLIT SEQUENCE NUMBER */
node_seq_t page_get_ssn_id(const page_t *page);
