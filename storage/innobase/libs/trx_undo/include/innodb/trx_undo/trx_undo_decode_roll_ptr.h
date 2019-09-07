#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

void trx_undo_decode_roll_ptr(
    roll_ptr_t roll_ptr, /*!< in: roll pointer */
    ibool *is_insert,    /*!< out: TRUE if insert undo log */
    ulint *rseg_id,      /*!< out: rollback segment id */
    page_no_t *page_no,  /*!< out: page number */
    ulint *offset);       /*!< out: offset of the undo
                         entry within page */
