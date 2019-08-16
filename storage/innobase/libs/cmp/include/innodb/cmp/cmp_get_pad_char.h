#pragma once

#include <innodb/univ/univ.h>


struct dtype_t;

ulint cmp_get_pad_char(const dtype_t *type);
