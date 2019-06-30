#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/BlockSyncArrayMutex.h>

typedef BlockSyncArrayMutex::MutexType BlockWaitMutex;
