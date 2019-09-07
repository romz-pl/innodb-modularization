#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_undo_rec_t.h>

/** Returns the first undo record on the page in the specified undo log, or
NULL if none exists.
@param[in]	undo_page	undo log page
@param[in]	page_no		undo log header page number
@param[in]	offset		undo log header offset on page
@return pointer to record, NULL if none */
trx_undo_rec_t *trx_undo_page_get_first_rec(page_t *undo_page,
                                            page_no_t page_no, ulint offset);
