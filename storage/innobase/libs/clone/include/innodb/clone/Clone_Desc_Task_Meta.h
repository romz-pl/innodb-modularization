#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Desc_Header.h>
#include <innodb/clone/Clone_Task_Meta.h>
#include <innodb/memory/mem_heap_t.h>

/** CLONE_DESC_TASK_METADATA: Descriptor for a task for clone operation.
A task is used by exactly one thread */
struct Clone_Desc_Task_Meta {
  /** Descriptor header */
  Clone_Desc_Header m_header;

  /** Task information */
  Clone_Task_Meta m_task_meta;

  /** Initialize header
  @param[in]	version	descriptor version */
  void init_header(uint version);

  /** Serialize the descriptor. Caller should pass
  the length if allocated.
  @param[out]	desc_task	serialized descriptor
  @param[in,out]	len		length of serialized descriptor
  @param[in]	heap		heap for allocating memory */
  void serialize(byte *&desc_task, uint &len, mem_heap_t *heap);

  /** Deserialize the descriptor.
  @param[in]	desc_task	serialized descriptor
  @param[in]	desc_len	descriptor length
  @return true, if successful. */
  bool deserialize(const byte *desc_task, uint desc_len);
};
