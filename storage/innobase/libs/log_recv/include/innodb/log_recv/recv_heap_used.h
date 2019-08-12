#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
size_t recv_heap_used();
#else  /* !UNIV_HOTBACKUP */
size_t meb_heap_used();
#endif /* !UNIV_HOTBACKUP */
