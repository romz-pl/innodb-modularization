#pragma once

#include <innodb/univ/univ.h>

/** Clone Handle State */
enum Clone_Handle_State {
  CLONE_STATE_INIT = 1,
  CLONE_STATE_ACTIVE,
  CLONE_STATE_IDLE,
  CLONE_STATE_ABORT
};
