#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>



struct lock_t;

typedef UT_LIST_BASE_NODE_T(lock_t) trx_lock_list_t;
