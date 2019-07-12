#pragma once

#include <innodb/univ/univ.h>

class page_size_t;

ulint fsp_header_get_encryption_progress_offset(
    const page_size_t &page_size);
