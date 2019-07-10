#include <innodb/ioasync/srv_shutdown_state.h>

/** At a shutdown this value climbs from SRV_SHUTDOWN_NONE to
SRV_SHUTDOWN_CLEANUP and then to SRV_SHUTDOWN_LAST_PHASE, and so on */
srv_shutdown_t srv_shutdown_state = SRV_SHUTDOWN_NONE;
