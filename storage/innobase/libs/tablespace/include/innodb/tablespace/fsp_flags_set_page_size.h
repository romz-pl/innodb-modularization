#pragma once

#include <innodb/univ/univ.h>

class page_size_t;

uint32_t fsp_flags_set_page_size(uint32_t flags, const page_size_t &page_size);
