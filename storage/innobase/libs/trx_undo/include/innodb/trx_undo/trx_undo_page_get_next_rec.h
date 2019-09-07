#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_undo_rec_t.h>

/** Returns the next undo log record on the page in the specified log, or
NULL if none exists.
@param[in]	rec		undo log record
@param[in]	page_no		undo log header page number
@param[in]	offset		undo log header offset on page
@return pointer to record, NULL if none */
trx_undo_rec_t *trx_undo_page_get_next_rec(trx_undo_rec_t *rec,
                                           page_no_t page_no, ulint offset);
