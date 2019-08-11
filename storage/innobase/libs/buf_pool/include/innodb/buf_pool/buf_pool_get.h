#pragma once

#include <innodb/univ/univ.h>

struct buf_pool_t;
class page_id_t;
buf_pool_t *buf_pool_get(const page_id_t &page_id);
