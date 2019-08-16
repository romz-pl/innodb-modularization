#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_get_offsets_func.h>
#include <innodb/univ/rec_t.h>

struct dict_index_t;

#ifdef UNIV_DEBUG

#define rec_get_offsets(rec, index, offsets, n, heap) \
  rec_get_offsets_func(rec, index, offsets, n, __FILE__, __LINE__, heap)

#else /* UNIV_DEBUG */

#define rec_get_offsets(rec, index, offsets, n, heap) \
  rec_get_offsets_func(rec, index, offsets, n, heap)

#endif /* UNIV_DEBUG */
