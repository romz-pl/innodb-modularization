#pragma once

#include <innodb/univ/univ.h>

/** TRUE when applying redo log records during crash recovery; FALSE
otherwise.  Note that this is FALSE while a background thread is
rolling back incomplete transactions. */
extern volatile bool recv_recovery_on;



