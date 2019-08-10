#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#define log_buffer_x_lock_own(log) log.sn_lock.x_own()

