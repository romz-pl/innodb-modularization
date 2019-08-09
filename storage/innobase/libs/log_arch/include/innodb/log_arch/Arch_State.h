#pragma once

#include <innodb/univ/univ.h>

/** Archiver system state.
Archiver state changes are triggered by client request to start or
stop archiving and system wide events like shutdown fatal error etc.
Following diagram shows the state transfer.

@startuml

  state ARCH_STATE_INIT
  state ARCH_STATE_ACTIVE
  state ARCH_STATE_PREPARE_IDLE
  state ARCH_STATE_IDLE
  state ARCH_STATE_ABORT

  [*] -down-> ARCH_STATE_INIT
  ARCH_STATE_INIT -down-> ARCH_STATE_ACTIVE : Start archiving
  ARCH_STATE_ACTIVE -right-> ARCH_STATE_PREPARE_IDLE : Stop archiving
  ARCH_STATE_PREPARE_IDLE -right-> ARCH_STATE_IDLE : All data archived
  ARCH_STATE_IDLE -down-> ARCH_STATE_ABORT : Shutdown or Fatal Error
  ARCH_STATE_PREPARE_IDLE --> ARCH_STATE_ACTIVE : Resume archiving
  ARCH_STATE_IDLE --> ARCH_STATE_ACTIVE : Start archiving
  ARCH_STATE_ABORT -down-> [*]

@enduml */
enum Arch_State {
  /** Archiver is initialized */
  ARCH_STATE_INIT = 0,

  /** Archiver is active and archiving data */
  ARCH_STATE_ACTIVE,

  /** Archiver is processing last data chunks before idle state */
  ARCH_STATE_PREPARE_IDLE,

  /** Archiver is idle */
  ARCH_STATE_IDLE,

  /** Archiver is aborted */
  ARCH_STATE_ABORT
};
