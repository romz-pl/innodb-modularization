#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/ib_mutex_t.h>

#ifndef UNIV_HOTBACKUP

typedef ib_mutex_t DictSysMutex;

#endif /* !UNIV_HOTBACKUP */
