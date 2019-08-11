#pragma once

#include <innodb/univ/univ.h>

struct buf_pool_t;
ulint buf_pool_index(const buf_pool_t *buf_pool);
