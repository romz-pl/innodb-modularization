#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;
class page_id_t;
void buf_block_set_file_page(buf_block_t *block, const page_id_t &page_id);
