#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
void buf_page_set_sticky(buf_page_t *bpage);
