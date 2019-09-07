#pragma once

#include <innodb/univ/univ.h>

class Space_Ids;

/** Build a list of unique undo tablespaces found in the TRX_SYS page.
Do not count the system tablespace. The vector will be sorted on space id.
@param[in,out]	spaces_to_open		list of undo tablespaces found. */
void trx_rseg_get_n_undo_tablespaces(Space_Ids *spaces_to_open);
