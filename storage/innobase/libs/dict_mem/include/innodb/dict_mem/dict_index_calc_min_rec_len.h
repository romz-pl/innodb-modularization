#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

/** Calculates the minimum record length in an index. */
ulint dict_index_calc_min_rec_len(const dict_index_t *index) /*!< in: index */
    MY_ATTRIBUTE((warn_unused_result));
