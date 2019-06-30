#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>
#include <innodb/sync_rw/rw_lock_t.h>

typedef UT_LIST_BASE_NODE_T(rw_lock_t) rw_lock_list_t;
