#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_offs_set_n_alloc.h>

#define rec_offs_init(offsets) \
  rec_offs_set_n_alloc(offsets, (sizeof offsets) / sizeof *offsets)
