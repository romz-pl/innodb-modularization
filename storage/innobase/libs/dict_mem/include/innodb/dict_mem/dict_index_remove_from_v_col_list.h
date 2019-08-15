#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

/** Clears the virtual column's index list before index is being freed.
@param[in]  index   Index being freed */
void dict_index_remove_from_v_col_list(dict_index_t *index);
