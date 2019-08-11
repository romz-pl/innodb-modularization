#pragma once

#include <innodb/univ/univ.h>

struct buf_pools_list_size_t;
void buf_get_total_list_size_in_bytes(buf_pools_list_size_t *buf_pools_list_size);
