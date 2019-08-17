#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_zip_stat_per_index_t.h>

/** Statistics on compression, indexed by dict_index_t::id */
extern page_zip_stat_per_index_t page_zip_stat_per_index;
