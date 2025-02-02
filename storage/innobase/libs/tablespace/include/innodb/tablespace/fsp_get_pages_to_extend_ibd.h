#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
class page_size_t;

page_no_t fsp_get_pages_to_extend_ibd(const page_size_t &page_size,
                                      page_no_t size);
