#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_System_State.h>

#include <vector>
#include <atomic>

using Clone_Sys_State = std::atomic<Clone_System_State>;
