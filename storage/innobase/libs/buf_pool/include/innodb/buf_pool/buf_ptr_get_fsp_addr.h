#pragma once

#include <innodb/univ/univ.h>


struct fil_addr_t;

void buf_ptr_get_fsp_addr(const void *ptr, space_id_t *space, fil_addr_t *addr);
