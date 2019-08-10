#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Desc_Header.h>
#include <innodb/clone/Snapshot_State.h>
#include <innodb/memory/mem_heap_t.h>

/** CLONE_DESC_STATE: Descriptor for current snapshot state */
struct Clone_Desc_State {
  /** Descriptor header */
  Clone_Desc_Header m_header;

  /** Current snapshot State */
  Snapshot_State m_state;

  /** Task identifier */
  uint m_task_index;

  /** Number of chunks in current state */
  uint m_num_chunks;

  /** Number of files in current state */
  uint m_num_files;

  /** Number of estimated bytes to transfer */
  uint64_t m_estimate;

  /** If start processing state */
  bool m_is_start;

  /** State transfer Acknowledgement */
  bool m_is_ack;

  /** Initialize header
  @param[in]	version	descriptor version */
  void init_header(uint version);

  /** Serialize the descriptor. Caller should pass
  the length if allocated.
  @param[out]	desc_state	serialized descriptor
  @param[in,out]	len		length of serialized descriptor
  @param[in]	heap		heap for allocating memory */
  void serialize(byte *&desc_state, uint &len, mem_heap_t *heap);

  /** Deserialize the descriptor.
  @param[in]	desc_state	serialized descriptor
  @param[in]	desc_len	descriptor length
  @return true, if successful. */
  bool deserialize(const byte *desc_state, uint desc_len);
};
