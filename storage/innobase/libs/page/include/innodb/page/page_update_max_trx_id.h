#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;
struct buf_block_t;
struct mtr_t;

/** Sets the max trx id field value if trx_id is bigger than the previous
value.
@param[in,out]	block		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	trx_id		transaction id
@param[in,out]	mtr		mini-transaction */
void page_update_max_trx_id(buf_block_t *block, page_zip_des_t *page_zip,
                            trx_id_t trx_id, mtr_t *mtr);
