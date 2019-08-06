#pragma once

#include <innodb/univ/univ.h>


/** CPU cache line size */
#ifdef __powerpc__
#define INNOBASE_CACHE_LINE_SIZE 128
#else
#define INNOBASE_CACHE_LINE_SIZE 64
#endif /* __powerpc__ */
