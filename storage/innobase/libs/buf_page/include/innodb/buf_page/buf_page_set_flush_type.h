#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_page/buf_flush_t.h>

class buf_page_t;
void buf_page_set_flush_type(buf_page_t *bpage, buf_flush_t flush_type);
