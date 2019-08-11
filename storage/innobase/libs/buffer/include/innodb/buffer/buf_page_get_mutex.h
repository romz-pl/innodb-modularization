#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_block/BPageMutex.h>

class buf_page_t;
BPageMutex *buf_page_get_mutex(const buf_page_t *bpage);
