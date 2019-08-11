#include <innodb/log_chkp/srv_log_checkpoint_every.h>

#include <innodb/log_types/flags.h>

/** Number of milliseconds every which a periodical checkpoint is written
by the log checkpointer thread (unless periodical checkpoints are disabled,
which is a case during initial phase of startup). */
ulong srv_log_checkpoint_every = INNODB_LOG_CHECKPOINT_EVERY_DEFAULT;
