#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Snapshot_State.h>
#include <innodb/clone/Clone_Desc_Header.h>
#include <innodb/memory/mem_heap_t.h>

struct Chunk_Info;


/** CLONE_DESC_LOCATOR: Descriptor for a task for clone operation.
A task is used by exactly one thread */
struct Clone_Desc_Locator {
  /** Descriptor header */
  Clone_Desc_Header m_header;

  /** Unique identifier for a clone operation. */
  uint64_t m_clone_id;

  /** Unique identifier for a clone snapshot. */
  uint64_t m_snapshot_id;

  /** Index in clone array for fast reference. */
  uint32_t m_clone_index;

  /** Current snapshot State */
  Snapshot_State m_state;

  /** Sub-state information: metadata transferred */
  bool m_metadata_transferred;

  /** Initialize clone locator.
  @param[in]	id	Clone identifier
  @param[in]	snap_id	Snapshot identifier
  @param[in]	state	snapshot state
  @param[in]	version	Descriptor version
  @param[in]	index	clone index */
  void init(ib_uint64_t id, ib_uint64_t snap_id, Snapshot_State state,
            uint version, uint index);

  /** Check if the passed locator matches the current one.
  @param[in]	other_desc	input locator descriptor
  @return true if matches */
  bool match(Clone_Desc_Locator *other_desc);

  /** Serialize the descriptor. Caller should pass
  the length if allocated.
  @param[out]	desc_loc	serialized descriptor
  @param[in,out]	len		length of serialized descriptor
  @param[in]	chunk_info	chunk information to serialize
  @param[in]	heap		heap for allocating memory */
  void serialize(byte *&desc_loc, uint &len, Chunk_Info *chunk_info,
                 mem_heap_t *heap);

  /** Deserialize the descriptor.
  @param[in]	desc_loc	serialized locator
  @param[in]	desc_len	locator length
  @param[in,out]	chunk_info	chunk information */
  void deserialize(const byte *desc_loc, uint desc_len, Chunk_Info *chunk_info);
};
