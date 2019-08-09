#pragma once

#include <innodb/univ/univ.h>

#include <functional>

/** Callback function to check if we need to wait for flush archiver to flush
more blocks */
using Page_Wait_Flush_Archiver_Cbk = std::function<bool(void)>;
