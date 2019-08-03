#pragma once

#include <innodb/univ/univ.h>

/** Calculates the space reserved for directory slots of a given
 number of records. The exact value is a fraction number
 n * PAGE_DIR_SLOT_SIZE / PAGE_DIR_SLOT_MIN_N_OWNED, and it is
 rounded upwards to an integer. */
ulint page_dir_calc_reserved_space(ulint n_recs); /*!< in: number of records */
