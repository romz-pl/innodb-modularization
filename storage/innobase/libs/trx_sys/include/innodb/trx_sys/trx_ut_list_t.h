#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>

struct trx_t;

typedef UT_LIST_BASE_NODE_T(trx_t) trx_ut_list_t;
