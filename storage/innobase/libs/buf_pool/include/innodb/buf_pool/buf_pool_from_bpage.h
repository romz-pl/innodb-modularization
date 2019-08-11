#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
struct buf_pool_t;
buf_pool_t *buf_pool_from_bpage(const buf_page_t *bpage);
