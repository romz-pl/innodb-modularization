#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;
void buf_block_buf_fix_inc_func(
#ifdef UNIV_DEBUG
    const char *file,   /*!< in: file name */
    ulint line,         /*!< in: line */
#endif                  /* UNIV_DEBUG */
    buf_block_t *block);
