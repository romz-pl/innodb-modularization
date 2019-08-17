#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_types/index_id_t.h>
#include <innodb/page/page_zip_stat_t.h>
#include <innodb/allocator/ut_allocator.h>


#include <map>

/** Compression statistics types */
typedef std::map<index_id_t, page_zip_stat_t, std::less<index_id_t>,
                 ut_allocator<std::pair<const index_id_t, page_zip_stat_t>>>
    page_zip_stat_per_index_t;
