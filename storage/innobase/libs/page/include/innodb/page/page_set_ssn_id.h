#pragma once

#include <innodb/univ/univ.h>

#include <innodb/gis_type/node_seq_t.h>

struct page_zip_des_t;
struct buf_block_t;
struct mtr_t;

/** Sets the RTREE SPLIT SEQUENCE NUMBER field value
@param[in,out]	block		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	ssn_id		split sequence id
@param[in,out]	mtr		mini-transaction */
void page_set_ssn_id(buf_block_t *block, page_zip_des_t *page_zip,
                     node_seq_t ssn_id, mtr_t *mtr);
