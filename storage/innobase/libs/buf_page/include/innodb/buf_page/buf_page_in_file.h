#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
ibool buf_page_in_file(const buf_page_t *bpage);
