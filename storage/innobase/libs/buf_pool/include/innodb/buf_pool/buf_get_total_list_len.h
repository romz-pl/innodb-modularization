#pragma once

#include <innodb/univ/univ.h>

void buf_get_total_list_len(ulint *LRU_len, ulint *free_len, ulint *flush_list_len);
