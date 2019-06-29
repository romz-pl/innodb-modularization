#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/UT_MUTEX_TYPE.h>
#include <innodb/sync_mutex/TTASFutexMutex.h>
#include <innodb/sync_policy/GenericPolicy.h>

#ifndef UNIV_LIBRARY
#ifdef HAVE_IB_LINUX_FUTEX

    UT_MUTEX_TYPE(TTASFutexMutex, GenericPolicy, FutexMutex)

#endif /* HAVE_IB_LINUX_FUTEX */
#endif
