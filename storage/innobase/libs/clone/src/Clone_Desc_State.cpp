#include <innodb/clone/Clone_Desc_State.h>

#include <innodb/clone/flags.h>
#include <innodb/machine/data.h>
#include <innodb/memory/mem_heap_alloc.h>

/** Clone State: Snapshot state in 4 bytes */
static const uint CLONE_DESC_STATE_OFFSET = CLONE_DESC_HEADER_LEN;

/** Clone State: Task index in 4 bytes */
static const uint CLONE_DESC_TASK_OFFSET = CLONE_DESC_STATE_OFFSET + 4;

/** Clone State: Number of chunks in 4 bytes */
static const uint CLONE_DESC_STATE_NUM_CHUNKS = CLONE_DESC_TASK_OFFSET + 4;

/** Clone State: Number of files in 4 bytes */
static const uint CLONE_DESC_STATE_NUM_FILES = CLONE_DESC_STATE_NUM_CHUNKS + 4;

/** Clone State: Estimated nuber of bytes in 8 bytes */
static const uint CLONE_DESC_STATE_EST_BYTES = CLONE_DESC_STATE_NUM_FILES + 4;

/** Clone State: flags in 2 byte [max 16 flags] */
static const uint CLONE_DESC_STATE_FLAGS = CLONE_DESC_STATE_EST_BYTES + 8;

/** Clone State: Total length */
static const uint CLONE_DESC_STATE_LEN = CLONE_DESC_STATE_FLAGS + 2;

UNIV_INLINE bool DESC_CHECK_FLAG(ulint flag, ulint bit) {
  return (!!((flag & (1ULL << (bit - 1))) > 0));
}

UNIV_INLINE void DESC_SET_FLAG(ulint &flag, ulint bit) {
  flag |= static_cast<ulint>(1ULL << (bit - 1));
}

/** Clone State Flag: Start processing state */
static const uint CLONE_DESC_STATE_FLAG_START = 1;

/** Clone State Flag: Acknowledge processing state */
static const uint CLONE_DESC_STATE_FLAG_ACK = 2;





void Clone_Desc_State::init_header(uint version) {
  m_header.m_version = version;

  m_header.m_length = CLONE_DESC_STATE_LEN;

  m_header.m_type = CLONE_DESC_STATE;
}

void Clone_Desc_State::serialize(byte *&desc_state, uint &len,
                                 mem_heap_t *heap) {
  /* Allocate descriptor if needed. */
  if (desc_state == nullptr) {
    len = m_header.m_length;
    desc_state = static_cast<byte *>(mem_heap_alloc(heap, len));
  } else {
    ut_ad(len >= m_header.m_length);
    len = m_header.m_length;
  }

  m_header.serialize(desc_state);

  mach_write_to_4(desc_state + CLONE_DESC_STATE_OFFSET, m_state);
  mach_write_to_4(desc_state + CLONE_DESC_TASK_OFFSET, m_task_index);

  mach_write_to_4(desc_state + CLONE_DESC_STATE_NUM_CHUNKS, m_num_chunks);
  mach_write_to_4(desc_state + CLONE_DESC_STATE_NUM_FILES, m_num_files);
  mach_write_to_8(desc_state + CLONE_DESC_STATE_EST_BYTES, m_estimate);

  ulint state_flags = 0;

  if (m_is_start) {
    DESC_SET_FLAG(state_flags, CLONE_DESC_STATE_FLAG_START);
  }

  if (m_is_ack) {
    DESC_SET_FLAG(state_flags, CLONE_DESC_STATE_FLAG_ACK);
  }

  mach_write_to_2(desc_state + CLONE_DESC_STATE_FLAGS, state_flags);
}

bool Clone_Desc_State::deserialize(const byte *desc_state, uint desc_len) {
  /* Deserialize the header and validate type and length. */
  if (desc_len < CLONE_DESC_STATE_LEN ||
      !m_header.deserialize(desc_state, desc_len) ||
      m_header.m_type != CLONE_DESC_STATE) {
    return (false);
  }

  uint int_type;
  int_type = mach_read_from_4(desc_state + CLONE_DESC_STATE_OFFSET);

  m_state = static_cast<Snapshot_State>(int_type);

  m_task_index = mach_read_from_4(desc_state + CLONE_DESC_TASK_OFFSET);

  m_num_chunks = mach_read_from_4(desc_state + CLONE_DESC_STATE_NUM_CHUNKS);
  m_num_files = mach_read_from_4(desc_state + CLONE_DESC_STATE_NUM_FILES);
  m_estimate = mach_read_from_8(desc_state + CLONE_DESC_STATE_EST_BYTES);

  auto state_flags =
      static_cast<ulint>(mach_read_from_2(desc_state + CLONE_DESC_STATE_FLAGS));

  m_is_start = DESC_CHECK_FLAG(state_flags, CLONE_DESC_STATE_FLAG_START);

  m_is_ack = DESC_CHECK_FLAG(state_flags, CLONE_DESC_STATE_FLAG_ACK);

  return (true);
}
