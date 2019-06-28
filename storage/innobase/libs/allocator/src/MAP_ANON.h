#pragma once

#include <innodb/univ/univ.h>

/* FreeBSD for example has only MAP_ANON, Linux has MAP_ANONYMOUS and
MAP_ANON but MAP_ANON is marked as deprecated */
#if defined(MAP_ANONYMOUS)
#define OS_MAP_ANON MAP_ANONYMOUS
#elif defined(MAP_ANON)
#define OS_MAP_ANON MAP_ANON
#endif
