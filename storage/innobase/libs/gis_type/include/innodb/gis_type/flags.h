#pragma once

#include <innodb/univ/univ.h>

/* Maximum index level for R-Tree, this is consistent with BTR_MAX_LEVELS */
#define RTR_MAX_LEVELS 100

/* Number of pages we latch at leaf level when there is possible Tree
modification (split, shrink), we always latch left, current
and right pages */
#define RTR_LEAF_LATCH_NUM 3
