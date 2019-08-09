#pragma once

#include <innodb/univ/univ.h>

/** Archived data block state.
A data block is a block in memory that holds dirty page IDs before persisting
into disk. Shown below is the state transfer diagram for a data block.

@startuml

  state ARCH_BLOCK_INIT
  state ARCH_BLOCK_ACTIVE
  state ARCH_BLOCK_READY_TO_FLUSH
  state ARCH_BLOCK_FLUSHED

  [*] -down-> ARCH_BLOCK_INIT
  ARCH_BLOCK_INIT -> ARCH_BLOCK_ACTIVE : Writing page ID
  ARCH_BLOCK_ACTIVE -> ARCH_BLOCK_READY_TO_FLUSH : Block is full
  ARCH_BLOCK_READY_TO_FLUSH -> ARCH_BLOCK_FLUSHED : Block is flushed
  ARCH_BLOCK_FLUSHED --> ARCH_BLOCK_ACTIVE : Writing page ID
  ARCH_BLOCK_FLUSHED -down-> [*]

@enduml */
enum Arch_Blk_State {
  /** Data block is initialized */
  ARCH_BLOCK_INIT = 0,

  /** Data block is active and having data */
  ARCH_BLOCK_ACTIVE,

  /** Data block is full but not flushed to disk */
  ARCH_BLOCK_READY_TO_FLUSH,

  /** Data block is flushed and can be reused */
  ARCH_BLOCK_FLUSHED
};
