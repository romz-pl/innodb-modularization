#include <innodb/clone/Clone_Desc_Data.h>

#include <innodb/clone/flags.h>
#include <innodb/machine/data.h>
#include <innodb/memory/mem_heap_alloc.h>


/** Clone Data: Snapshot state in 4 bytes */
static const uint CLONE_DATA_STATE_OFFSET = CLONE_DESC_HEADER_LEN;

/** Clone Data: Task index in 4 bytes */
static const uint CLONE_DATA_TASK_INDEX_OFFSET = CLONE_DATA_STATE_OFFSET + 4;

/** Clone Data: Current chunk number in 4 bytes */
static const uint CLONE_DATA_TASK_CHUNK_OFFSET =
    CLONE_DATA_TASK_INDEX_OFFSET + 4;

/** Clone Data: Current block number in 4 bytes */
static const uint CLONE_DATA_TASK_BLOCK_OFFSET =
    CLONE_DATA_TASK_CHUNK_OFFSET + 4;

/** Clone Data: Data file index in 4 bytes */
static const uint CLONE_DATA_FILE_IDX_OFFSET = CLONE_DATA_TASK_BLOCK_OFFSET + 4;

/** Clone Data: Data length in 4 bytes */
static const uint CLONE_DATA_LEN_OFFSET = CLONE_DATA_FILE_IDX_OFFSET + 4;

/** Clone Data: Data file offset in 8 bytes */
static const uint CLONE_DATA_FOFF_OFFSET = CLONE_DATA_LEN_OFFSET + 4;

/** Clone Data: Updated file size in 8 bytes */
static const uint CLONE_DATA_FILE_SIZE_OFFSET = CLONE_DATA_FOFF_OFFSET + 8;

/** Clone Data: Total length */
static const uint CLONE_DESC_DATA_LEN = CLONE_DATA_FILE_SIZE_OFFSET + 8;

void Clone_Desc_Data::init_header(uint version) {
  m_header.m_version = version;

  m_header.m_length = CLONE_DESC_DATA_LEN;

  m_header.m_type = CLONE_DESC_DATA;
}

void Clone_Desc_Data::serialize(byte *&desc_data, uint &len, mem_heap_t *heap) {
  /* Allocate descriptor if needed. */
  if (desc_data == nullptr) {
    len = m_header.m_length;
    desc_data = static_cast<byte *>(mem_heap_alloc(heap, len));
  } else {
    ut_ad(len >= m_header.m_length);
    len = m_header.m_length;
  }

  m_header.serialize(desc_data);

  mach_write_to_4(desc_data + CLONE_DATA_STATE_OFFSET, m_state);

  mach_write_to_4(desc_data + CLONE_DATA_TASK_INDEX_OFFSET,
                  m_task_meta.m_task_index);
  mach_write_to_4(desc_data + CLONE_DATA_TASK_CHUNK_OFFSET,
                  m_task_meta.m_chunk_num);
  mach_write_to_4(desc_data + CLONE_DATA_TASK_BLOCK_OFFSET,
                  m_task_meta.m_block_num);

  mach_write_to_4(desc_data + CLONE_DATA_FILE_IDX_OFFSET, m_file_index);
  mach_write_to_4(desc_data + CLONE_DATA_LEN_OFFSET, m_data_len);
  mach_write_to_8(desc_data + CLONE_DATA_FOFF_OFFSET, m_file_offset);
  mach_write_to_8(desc_data + CLONE_DATA_FILE_SIZE_OFFSET, m_file_size);
}

bool Clone_Desc_Data::deserialize(const byte *desc_data, uint desc_len) {
  /* Deserialize the header and validate type and length. */
  if (desc_len < CLONE_DESC_DATA_LEN ||
      !m_header.deserialize(desc_data, desc_len) ||
      m_header.m_type != CLONE_DESC_DATA) {
    return (false);
  }

  uint int_type;
  int_type = mach_read_from_4(desc_data + CLONE_DATA_STATE_OFFSET);

  m_state = static_cast<Snapshot_State>(int_type);

  m_task_meta.m_task_index =
      mach_read_from_4(desc_data + CLONE_DATA_TASK_INDEX_OFFSET);

  m_task_meta.m_chunk_num =
      mach_read_from_4(desc_data + CLONE_DATA_TASK_CHUNK_OFFSET);

  m_task_meta.m_block_num =
      mach_read_from_4(desc_data + CLONE_DATA_TASK_BLOCK_OFFSET);

  m_file_index = mach_read_from_4(desc_data + CLONE_DATA_FILE_IDX_OFFSET);
  m_data_len = mach_read_from_4(desc_data + CLONE_DATA_LEN_OFFSET);
  m_file_offset = mach_read_from_8(desc_data + CLONE_DATA_FOFF_OFFSET);
  m_file_size = mach_read_from_8(desc_data + CLONE_DATA_FILE_SIZE_OFFSET);

  return (true);
}
