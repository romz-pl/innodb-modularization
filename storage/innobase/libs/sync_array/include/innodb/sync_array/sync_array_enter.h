#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/mutex_enter.h>

#define sync_array_enter(a) mutex_enter(&(a)->mutex)
