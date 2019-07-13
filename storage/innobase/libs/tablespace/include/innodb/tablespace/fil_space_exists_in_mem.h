#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

#include <innodb/memory/mem_heap_t.h>

bool fil_space_exists_in_mem(space_id_t space_id, const char *name,
                             bool print_err, bool adjust_space,
                             mem_heap_t *heap, table_id_t table_id)
    MY_ATTRIBUTE((warn_unused_result));
