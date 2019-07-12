#pragma once

#include <innodb/univ/univ.h>

class page_size_t;

ulint xdes_calc_descriptor_index(const page_size_t &page_size, ulint offset);
