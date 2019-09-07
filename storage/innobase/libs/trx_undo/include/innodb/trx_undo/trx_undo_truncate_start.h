#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

struct trx_rseg_t;

/** Truncate the head of an undo log.
NOTE that only whole pages are freed; the header page is not
freed, but emptied, if all the records there are below the limit.
@param[in,out]	rseg		rollback segment
@param[in]	hdr_page_no	header page number
@param[in]	hdr_offset	header offset on the page
@param[in]	limit		first undo number to preserve
(everything below the limit will be truncated) */
void trx_undo_truncate_start(trx_rseg_t *rseg, page_no_t hdr_page_no,
                             ulint hdr_offset, undo_no_t limit);
