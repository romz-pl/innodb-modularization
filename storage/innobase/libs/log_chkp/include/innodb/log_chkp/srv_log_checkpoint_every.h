#pragma once

#include <innodb/univ/univ.h>

/** Number of milliseconds every which a periodical checkpoint is written
by the log checkpointer thread (unless periodical checkpoints are disabled,
which is a case during initial phase of startup). */
extern ulong srv_log_checkpoint_every;
