#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>
#include <innodb/trx_trx/TrxVersion.h>

#include <list>

typedef std::list<TrxVersion, ut_allocator<TrxVersion>> hit_list_t;
