#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

class buf_page_t;
lsn_t buf_page_get_newest_modification(const buf_page_t *bpage);
