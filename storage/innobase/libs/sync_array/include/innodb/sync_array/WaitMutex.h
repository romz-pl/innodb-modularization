#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/SyncArrayMutex.h>

typedef SyncArrayMutex::MutexType WaitMutex;
