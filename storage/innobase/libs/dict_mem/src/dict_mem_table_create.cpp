#include <innodb/dict_mem/dict_mem_table_create.h>

#include <innodb/dict_mem/dict_v_col_t.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_tf2_is_valid.h>
#include <innodb/memory/mem_heap_create.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/dict_mem/lock_table_lock_list_init.h>
#include <innodb/string/mem_strdup.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/dict_mem/dict_table_mutex_create_lazy.h>
#include <innodb/dict_mem/dict_table_stats_latch_create.h>
#include <innodb/dict_mem/dict_table_autoinc_create_lazy.h>
#include <innodb/dict_mem/dict_table_has_fts_index.h>


class fts_t;
fts_t *fts_create(dict_table_t *table);


