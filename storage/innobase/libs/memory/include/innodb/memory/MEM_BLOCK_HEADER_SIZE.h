#pragma once

#include <innodb/univ/univ.h>

#include <innodb/align/ut_calc_align.h>
#include <innodb/memory/mem_block_info_t.h>

/* Header size for a memory heap block */
#define MEM_BLOCK_HEADER_SIZE \
  ut_calc_align(sizeof(mem_block_info_t), UNIV_MEM_ALIGNMENT)
