#pragma once

#include <innodb/univ/univ.h>

/** TRUE when applying redo log records during crash recovery; FALSE
otherwise.  Note that this is FALSE while a background thread is
rolling back incomplete transactions. */
extern volatile bool recv_recovery_on;


/** Returns true if recovery is currently running.
@return recv_recovery_on */
inline
bool recv_recovery_is_on() {
    return (recv_recovery_on);
}
