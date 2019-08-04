#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct dtype_t;

void dtype_set_mblen(dtype_t *type);

#else

#define dtype_set_mblen(type) (void)0

#endif
