#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_id_t.h>

ulint recv_read_in_area(const page_id_t &page_id);
