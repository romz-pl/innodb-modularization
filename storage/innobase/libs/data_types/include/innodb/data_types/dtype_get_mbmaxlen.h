#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct dtype_t;

ulint dtype_get_mbmaxlen(const dtype_t *type);

#endif
