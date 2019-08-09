#pragma once

#include <innodb/univ/univ.h>

/** Archiver client state.
Archiver clients request archiving for specific interval using
the start and stop interfaces. During this time the client is
attached to global Archiver system. A client copies archived
data for the interval after calling stop. System keeps the data
till the time client object is destroyed.

@startuml

  state ARCH_CLIENT_STATE_INIT
  state ARCH_CLIENT_STATE_STARTED
  state ARCH_CLIENT_STATE_STOPPED

  [*] -down-> ARCH_CLIENT_STATE_INIT
  ARCH_CLIENT_STATE_INIT -down-> ARCH_CLIENT_STATE_STARTED : Attach and start \
  archiving
  ARCH_CLIENT_STATE_STARTED -right-> ARCH_CLIENT_STATE_STOPPED : Stop \
  archiving
  ARCH_CLIENT_STATE_STOPPED -down-> [*] : Detach client

@enduml */
enum Arch_Client_State {
  /** Client is initialized */
  ARCH_CLIENT_STATE_INIT = 0,

  /** Archiving started by client */
  ARCH_CLIENT_STATE_STARTED,

  /** Archiving stopped by client */
  ARCH_CLIENT_STATE_STOPPED
};
