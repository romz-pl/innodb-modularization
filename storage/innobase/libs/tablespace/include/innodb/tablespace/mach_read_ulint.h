#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>

uint32_t mach_read_ulint(const byte *ptr, mlog_id_t type);
