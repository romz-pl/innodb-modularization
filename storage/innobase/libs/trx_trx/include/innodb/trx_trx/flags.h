#pragma once

#include <innodb/univ/univ.h>

const ulint MAX_DETAILED_ERROR_LEN = 256;

/* Maximum length of a string that can be returned by
trx_get_que_state_str(). */
#define TRX_QUE_STATE_STR_MAX_LEN 12 /* "ROLLING BACK" */


/* Treatment of duplicate values (trx->duplicates; for example, in inserts).
Multiple flags can be combined with bitwise OR. */
#define TRX_DUP_IGNORE 1  /* duplicate rows are to be updated */
#define TRX_DUP_REPLACE 2 /* duplicate rows are to be replaced */



