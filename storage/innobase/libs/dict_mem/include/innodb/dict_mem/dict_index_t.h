#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>
#include <innodb/dict_types/id_name_t.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/dict_mem/rec_cache_t.h>
#include <innodb/dict_mem/zip_pad_info_t.h>
#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/dict_mem/dict_field_t.h>
#include <innodb/dict_mem/dict_col_t.h>

#include "include/mysql/plugin_ftparser.h"

struct dict_table_t;
struct dict_field_t;
struct last_ops_cur_t;
struct btr_search_t;
struct row_log_t;
struct trx_t;


namespace dd { class Spatial_reference_system; }
