#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
ibool buf_page_belongs_to_unzip_LRU(const buf_page_t *bpage);
