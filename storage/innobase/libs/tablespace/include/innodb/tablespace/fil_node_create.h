#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

struct fil_space_t;

char *fil_node_create(const char *name, page_no_t size, fil_space_t *space,
                      bool is_raw, bool atomic_write,
                      page_no_t max_pages = PAGE_NO_MAX) MY_ATTRIBUTE((warn_unused_result));
