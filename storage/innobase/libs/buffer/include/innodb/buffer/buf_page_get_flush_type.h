#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_flush_t.h>

class buf_page_t;
buf_flush_t buf_page_get_flush_type(const buf_page_t *bpage);
