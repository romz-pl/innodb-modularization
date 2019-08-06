#pragma once

#include <innodb/univ/univ.h>

enum mtr_state_t {
  MTR_STATE_INIT = 0,
  MTR_STATE_ACTIVE = 12231,
  MTR_STATE_COMMITTING = 56456,
  MTR_STATE_COMMITTED = 34676
};
