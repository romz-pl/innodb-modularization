#pragma once

#include <innodb/univ/univ.h>

#include <innodb/ioasync/srv_shutdown_t.h>

/** At a shutdown this value climbs from SRV_SHUTDOWN_NONE to
SRV_SHUTDOWN_CLEANUP and then to SRV_SHUTDOWN_LAST_PHASE, and so on */
extern srv_shutdown_t srv_shutdown_state;
