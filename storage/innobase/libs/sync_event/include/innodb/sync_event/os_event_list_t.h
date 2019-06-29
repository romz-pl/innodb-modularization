#pragma once

#include <innodb/univ/univ.h>
#include <innodb/allocator/ut_allocator.h>
#include <innodb/sync_event/os_event_t.h>

#include <list>

typedef std::list<os_event_t, ut_allocator<os_event_t>> os_event_list_t;
