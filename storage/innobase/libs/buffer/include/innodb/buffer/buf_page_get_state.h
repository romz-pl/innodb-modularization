#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_page/buf_page_state.h>

class buf_page_t;
buf_page_state buf_page_get_state(const buf_page_t *bpage);
