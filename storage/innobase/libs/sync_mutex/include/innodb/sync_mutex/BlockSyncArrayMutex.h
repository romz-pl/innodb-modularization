#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/UT_MUTEX_TYPE.h>
#include <innodb/sync_mutex/TTASEventMutex.h>
#include <innodb/sync_policy/BlockMutexPolicy.h>

#ifndef UNIV_LIBRARY

UT_MUTEX_TYPE(TTASEventMutex, BlockMutexPolicy, BlockSyncArrayMutex)

#endif
