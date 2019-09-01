#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Cancels a waiting lock request and releases possible other transactions
waiting behind it.
@param[in,out]	lock		Waiting lock request
@param[in]	use_fcfs	true -> use first come first served strategy */
void lock_cancel_waiting_and_release(lock_t *lock, bool use_fcfs);
