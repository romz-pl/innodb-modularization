#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/dyn_buf_t.h>

typedef dyn_buf_t<DYN_ARRAY_DATA_SIZE> mtr_buf_t;
