#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/UT_MUTEX_TYPE.h>
#include <innodb/sync_mutex/TTASEventMutex.h>
#include <innodb/sync_policy/GenericPolicy.h>

#ifndef UNIV_LIBRARY

UT_MUTEX_TYPE(TTASEventMutex, GenericPolicy, SyncArrayMutex)

#endif
