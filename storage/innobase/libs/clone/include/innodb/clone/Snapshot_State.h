#pragma once

#include <innodb/univ/univ.h>

/** Snapshot state transfer during clone.

Clone Type: HA_CLONE_BLOCKING
@startuml
  state CLONE_SNAPSHOT_INIT
  state CLONE_SNAPSHOT_FILE_COPY
  state CLONE_SNAPSHOT_DONE

  [*] -down-> CLONE_SNAPSHOT_INIT : Build snapshot
  CLONE_SNAPSHOT_INIT -right-> CLONE_SNAPSHOT_FILE_COPY
  CLONE_SNAPSHOT_FILE_COPY -right-> CLONE_SNAPSHOT_DONE
  CLONE_SNAPSHOT_DONE -down-> [*] : Destroy snapshot
@enduml

Clone Type: HA_CLONE_REDO
@startuml
  state CLONE_SNAPSHOT_REDO_COPY

  [*] -down-> CLONE_SNAPSHOT_INIT : Build snapshot
  CLONE_SNAPSHOT_INIT -right-> CLONE_SNAPSHOT_FILE_COPY : Start redo archiving
  CLONE_SNAPSHOT_FILE_COPY -right-> CLONE_SNAPSHOT_REDO_COPY
  CLONE_SNAPSHOT_REDO_COPY -right-> CLONE_SNAPSHOT_DONE
  CLONE_SNAPSHOT_DONE -down-> [*] : Destroy snapshot
@enduml

Clone Type: HA_CLONE_HYBRID
@startuml
  state CLONE_SNAPSHOT_PAGE_COPY

  [*] -down-> CLONE_SNAPSHOT_INIT : Build snapshot
  CLONE_SNAPSHOT_INIT -right-> CLONE_SNAPSHOT_FILE_COPY : Start page tracking
  CLONE_SNAPSHOT_FILE_COPY -right-> CLONE_SNAPSHOT_PAGE_COPY : Start redo \
  archiving
  CLONE_SNAPSHOT_PAGE_COPY -right-> CLONE_SNAPSHOT_REDO_COPY
  CLONE_SNAPSHOT_REDO_COPY -right> CLONE_SNAPSHOT_DONE
  CLONE_SNAPSHOT_DONE -down-> [*] : Destroy snapshot
@enduml

Clone Type: HA_CLONE_PAGE: Not implemented
*/
enum Snapshot_State {
  /** Invalid state */
  CLONE_SNAPSHOT_NONE = 0,

  /** Initialize state when snapshot object is created */
  CLONE_SNAPSHOT_INIT,

  /** Snapshot state while transferring files. */
  CLONE_SNAPSHOT_FILE_COPY,

  /** Snapshot state while transferring pages. */
  CLONE_SNAPSHOT_PAGE_COPY,

  /** Snapshot state while transferring redo. */
  CLONE_SNAPSHOT_REDO_COPY,

  /** Snapshot state at end after finishing transfer. */
  CLONE_SNAPSHOT_DONE
};
