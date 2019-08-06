#pragma once

#include <innodb/univ/univ.h>

struct mtr_memo_slot_t;

void memo_slot_release(mtr_memo_slot_t *slot);
