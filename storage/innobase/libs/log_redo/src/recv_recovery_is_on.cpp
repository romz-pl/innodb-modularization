#include <innodb/log_redo/recv_recovery_is_on.h>

#include <innodb/log_redo/recv_recovery_on.h>

/** Returns true if recovery is currently running.
@return recv_recovery_on */
bool recv_recovery_is_on() {
    return (recv_recovery_on);
}
