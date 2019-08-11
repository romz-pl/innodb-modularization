#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_write/srv_stats_t.h>

#ifndef UNIV_HOTBACKUP

/** Global counters */
extern srv_stats_t srv_stats;

#endif /* !UNIV_HOTBACKUP */
