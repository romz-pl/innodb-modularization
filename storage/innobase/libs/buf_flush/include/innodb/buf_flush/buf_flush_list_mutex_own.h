#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_mutex/mutex_own.h>

/** Test if flush list mutex is owned. */
#define buf_flush_list_mutex_own(b) mutex_own(&(b)->flush_list_mutex)

#endif
