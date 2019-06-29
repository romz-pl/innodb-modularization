#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_mutex/PolicyMutex.h>

/** Create a typedef using the MutexType<PolicyType>
@param[in]	M		Mutex type
@param[in]	P		Policy type
@param[in]	T		The resulting typedef alias */
#define UT_MUTEX_TYPE(M, P, T) typedef PolicyMutex<M<P>> T;
