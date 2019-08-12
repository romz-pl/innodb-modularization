#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_recv/recv_sys_t.h>

recv_sys_t::Space *recv_get_page_map(space_id_t space_id, bool create);
