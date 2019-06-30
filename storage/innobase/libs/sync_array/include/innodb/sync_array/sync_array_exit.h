#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/mutex_exit.h>

#define sync_array_exit(a) mutex_exit(&(a)->mutex)
