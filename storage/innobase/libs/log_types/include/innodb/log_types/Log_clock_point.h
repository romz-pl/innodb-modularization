#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/Log_clock.h>

/** Time point defined by the Log_clock. */
using Log_clock_point = std::chrono::time_point<Log_clock>;
