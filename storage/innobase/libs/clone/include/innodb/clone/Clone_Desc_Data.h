#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Desc_Header.h>
#include <innodb/clone/Snapshot_State.h>
#include <innodb/clone/Clone_Task_Meta.h>
#include <innodb/memory/mem_heap_t.h>

/** CLONE_DESC_DATA: Descriptor for data */
struct Clone_Desc_Data {
  /** Descriptor header */
  Clone_Desc_Header m_header;

  /** Current snapshot State */
  Snapshot_State m_state;

  /** Task information */
  Clone_Task_Meta m_task_meta;

  /** File identifier */
  uint32_t m_file_index;

  /** Data Length */
  uint32_t m_data_len;

  /** File offset for the data */
  uint64_t m_file_offset;

  /** Updated file size */
  uint64_t m_file_size;

  /** Initialize header
  @param[in]	version	descriptor version */
  void init_header(uint version);

  /** Serialize the descriptor. Caller should pass
  the length if allocated.
  @param[out]	desc_data	serialized descriptor
  @param[in,out]	len		length of serialized descriptor
  @param[in]	heap		heap for allocating memory */
  void serialize(byte *&desc_data, uint &len, mem_heap_t *heap);

  /** Deserialize the descriptor.
  @param[in]	desc_data	serialized descriptor
  @param[in]	desc_len	descriptor length
  @return true, if successful. */
  bool deserialize(const byte *desc_data, uint desc_len);
};
