#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_page/buf_io_fix.h>

class buf_page_t;
buf_io_fix buf_page_get_io_fix_unlocked(const buf_page_t *bpage);
