#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_HOTBACKUP

typedef rw_lock_t BPageLock;

#endif /* !UNIV_HOTBACKUP */
