#pragma once

#include <innodb/univ/univ.h>

/** Clone system state */
enum Clone_System_State {
  CLONE_SYS_INACTIVE,
  CLONE_SYS_ACTIVE,
  CLONE_SYS_ABORT
};
