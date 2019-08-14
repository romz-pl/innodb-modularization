#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

class page_size_t;

page_no_t xdes_calc_descriptor_page(const page_size_t &page_size, page_no_t offset);
