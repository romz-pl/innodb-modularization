#pragma once

#include <innodb/univ/univ.h>

struct buf_pool_t;
ulint buf_get_withdraw_depth(buf_pool_t *buf_pool);
