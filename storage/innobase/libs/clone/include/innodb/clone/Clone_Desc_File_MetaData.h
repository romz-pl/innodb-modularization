#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Desc_Header.h>
#include <innodb/clone/Snapshot_State.h>
#include <innodb/clone/Clone_File_Meta.h>
#include <innodb/memory/mem_heap_t.h>

/** CLONE_DESC_FILE_METADATA: Descriptor for file metadata */
struct Clone_Desc_File_MetaData {
  /** Descriptor header */
  Clone_Desc_Header m_header;

  /** Current snapshot State */
  Snapshot_State m_state;

  /** File metadata */
  Clone_File_Meta m_file_meta;

  /** Initialize header
  @param[in]	version	descriptor version */
  void init_header(uint version);

  /** Serialize the descriptor. Caller should pass
  the length if allocated.
  @param[out]	desc_file	serialized descriptor
  @param[in,out]	len		length of serialized descriptor
  @param[in]	heap		heap for allocating memory */
  void serialize(byte *&desc_file, uint &len, mem_heap_t *heap);

  /** Deserialize the descriptor.
  @param[in]	desc_file	serialized descriptor
  @param[in]	desc_len	descriptor length
  @return true, if successful. */
  bool deserialize(const byte *desc_file, uint desc_len);
};
