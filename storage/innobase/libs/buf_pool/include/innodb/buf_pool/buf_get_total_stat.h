#pragma once

#include <innodb/univ/univ.h>

struct buf_pool_stat_t;
void buf_get_total_stat(buf_pool_stat_t *tot_stat);
