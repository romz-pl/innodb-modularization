#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Create the file page for the rollback segment directory in an undo
tablespace. This function is called just after an undo tablespace is
created so the next page created here should by FSP_FSEG_DIR_PAGE_NUM.
@param[in]	space_id	Undo Tablespace ID
@param[in]	mtr		mtr */
void trx_rseg_array_create(space_id_t space_id, mtr_t *mtr);
