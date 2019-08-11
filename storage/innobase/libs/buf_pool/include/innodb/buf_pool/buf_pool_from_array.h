#pragma once

#include <innodb/univ/univ.h>

struct buf_pool_t;
buf_pool_t *buf_pool_from_array(ulint index);
