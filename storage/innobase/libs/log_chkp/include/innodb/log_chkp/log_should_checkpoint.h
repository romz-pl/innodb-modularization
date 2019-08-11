#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Checks if checkpoint should be written. Checks time elapsed since the last
checkpoint, age of the last checkpoint and if there was any extra request to
write the checkpoint (e.g. coming from log_make_latest_checkpoint()).
@return true if checkpoint should be written */
bool log_should_checkpoint(log_t &log);
