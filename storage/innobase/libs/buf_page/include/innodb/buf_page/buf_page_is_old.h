#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
ibool buf_page_is_old(const buf_page_t *bpage);
