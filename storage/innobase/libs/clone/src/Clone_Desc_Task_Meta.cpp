#include <innodb/clone/Clone_Desc_Task_Meta.h>

#include <innodb/assert/assert.h>
#include <innodb/clone/flags.h>
#include <innodb/machine/data.h>
#include <innodb/memory/mem_heap_alloc.h>

/** Task: Clone task index in 4 bytes */
static const uint CLONE_TASK_INDEX_OFFSET = CLONE_DESC_HEADER_LEN;

/** Task: Task chunk number in 4 bytes */
static const uint CLONE_TASK_CHUNK_OFFSET = CLONE_TASK_INDEX_OFFSET + 4;

/** Task: Task block number in 4 bytes */
static const uint CLONE_TASK_BLOCK_OFFSET = CLONE_TASK_CHUNK_OFFSET + 4;

/** Task: Total length */
static const uint CLONE_TASK_META_LEN = CLONE_TASK_BLOCK_OFFSET + 4;




/** Initialize header
@param[in]	version	descriptor version */
void Clone_Desc_Task_Meta::init_header(uint version) {
  m_header.m_version = version;

  m_header.m_length = CLONE_TASK_META_LEN;

  m_header.m_type = CLONE_DESC_TASK_METADATA;
}

void Clone_Desc_Task_Meta::serialize(byte *&desc_task, uint &len,
                                     mem_heap_t *heap) {
  if (desc_task == nullptr) {
    len = m_header.m_length;
    desc_task = static_cast<byte *>(mem_heap_alloc(heap, len));
  } else {
    ut_ad(len >= m_header.m_length);
    len = m_header.m_length;
  }

  m_header.serialize(desc_task);

  mach_write_to_4(desc_task + CLONE_TASK_INDEX_OFFSET,
                  m_task_meta.m_task_index);
  mach_write_to_4(desc_task + CLONE_TASK_CHUNK_OFFSET, m_task_meta.m_chunk_num);
  mach_write_to_4(desc_task + CLONE_TASK_BLOCK_OFFSET, m_task_meta.m_block_num);
}

bool Clone_Desc_Task_Meta::deserialize(const byte *desc_task, uint desc_len) {
  /* Deserialize the header and validate type and length. */
  if (desc_len < CLONE_TASK_META_LEN ||
      !m_header.deserialize(desc_task, desc_len) ||
      m_header.m_type != CLONE_DESC_TASK_METADATA) {
    return (false);
  }
  m_task_meta.m_task_index =
      mach_read_from_4(desc_task + CLONE_TASK_INDEX_OFFSET);
  m_task_meta.m_chunk_num =
      mach_read_from_4(desc_task + CLONE_TASK_CHUNK_OFFSET);
  m_task_meta.m_block_num =
      mach_read_from_4(desc_task + CLONE_TASK_BLOCK_OFFSET);
  return (true);
}

