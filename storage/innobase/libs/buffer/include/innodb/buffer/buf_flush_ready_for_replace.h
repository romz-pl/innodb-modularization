#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;

ibool buf_flush_ready_for_replace(buf_page_t *bpage);
