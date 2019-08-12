#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_recv/recv_addr_t.h>

recv_addr_t *recv_get_rec(space_id_t space_id, page_no_t page_no);
