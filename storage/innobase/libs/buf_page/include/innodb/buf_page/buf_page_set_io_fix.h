#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_page/buf_io_fix.h>

class buf_page_t;
void buf_page_set_io_fix(buf_page_t *bpage, buf_io_fix io_fix);
