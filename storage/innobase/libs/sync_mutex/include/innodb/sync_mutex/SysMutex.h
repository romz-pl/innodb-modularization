#pragma once

#include <innodb/sync_mutex/SysMutex.h>
#include <innodb/sync_mutex/UT_MUTEX_TYPE.h>
#include <innodb/sync_mutex/OSTrackMutex.h>
#include <innodb/sync_policy/GenericPolicy.h>

#ifdef UNIV_LIBRARY

    typedef OSMutex SysMutex;

#else

    UT_MUTEX_TYPE(OSTrackMutex, GenericPolicy, SysMutex)

#endif
