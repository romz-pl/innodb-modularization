#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/tablespace/xdes_t.h>

ibool xdes_get_bit(const xdes_t *descr, ulint bit, page_no_t offset);
