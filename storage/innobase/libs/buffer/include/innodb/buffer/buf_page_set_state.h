#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_page/buf_page_state.h>

class buf_page_t;
void buf_page_set_state(buf_page_t *bpage, buf_page_state state);
