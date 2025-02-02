#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/disk/page_size_t.h>

page_no_t fsp_get_extent_size_in_pages(const page_size_t &page_size);
