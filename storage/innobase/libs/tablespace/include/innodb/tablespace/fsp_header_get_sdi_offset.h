#pragma once

#include <innodb/univ/univ.h>

class page_size_t;

ulint fsp_header_get_sdi_offset(const page_size_t &page_size);
