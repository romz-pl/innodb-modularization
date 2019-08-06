#pragma once

#include <innodb/univ/univ.h>

#include <chrono>

/** Clock used to measure time spent in redo log (e.g. when flushing). */
using Log_clock = std::chrono::high_resolution_clock;
