#pragma once

#include <innodb/univ/univ.h>

class page_size_t;

uint32_t fsp_flags_init(const page_size_t &page_size, bool atomic_blobs,
                        bool has_data_dir, bool is_shared, bool is_temporary,
                        bool is_encrypted = false);
