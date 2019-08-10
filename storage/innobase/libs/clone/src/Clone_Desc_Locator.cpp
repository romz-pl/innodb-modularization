#include <innodb/clone/Clone_Desc_Locator.h>

#include <innodb/clone/flags.h>
#include <innodb/machine/data.h>
#include <innodb/clone/Chunk_Info.h>
#include <innodb/memory/mem_heap_alloc.h>


void Clone_Desc_Locator::init(ib_uint64_t id, ib_uint64_t snap_id,
                              Snapshot_State state, uint version, uint index) {
  m_header.m_version = version;

  m_header.m_length = CLONE_DESC_LOC_BASE_LEN;

  m_header.m_type = CLONE_DESC_LOCATOR;

  m_clone_id = id;
  m_snapshot_id = snap_id;

  m_clone_index = index;
  m_state = state;
  m_metadata_transferred = false;
}

bool Clone_Desc_Locator::match(Clone_Desc_Locator *other_desc) {
#ifdef UNIV_DEBUG
  Clone_Desc_Header *other_header = &other_desc->m_header;
#endif /* UNIV_DEBUG */

  if (other_desc->m_clone_id == m_clone_id &&
      other_desc->m_snapshot_id == m_snapshot_id) {
    ut_ad(m_header.m_version == other_header->m_version);
    return (true);
  }

  return (false);
}

void Clone_Desc_Locator::serialize(byte *&desc_loc, uint &len,
                                   Chunk_Info *chunk_info, mem_heap_t *heap) {
  if (chunk_info != nullptr) {
    auto chunk_len = static_cast<uint>(chunk_info->get_serialized_length(0));

    m_header.m_length += chunk_len;
  }

  if (desc_loc == nullptr) {
    len = m_header.m_length;
    desc_loc = static_cast<byte *>(mem_heap_alloc(heap, len));
  } else {
    ut_ad(len >= m_header.m_length);
    len = m_header.m_length;
  }

  m_header.serialize(desc_loc);

  mach_write_to_8(desc_loc + CLONE_LOC_CID_OFFSET, m_clone_id);
  mach_write_to_8(desc_loc + CLONE_LOC_SID_OFFSET, m_snapshot_id);

  mach_write_to_4(desc_loc + CLONE_LOC_IDX_OFFSET, m_clone_index);

  mach_write_to_1(desc_loc + CLONE_LOC_STATE_OFFSET,
                  static_cast<ulint>(m_state));

  ulint sub_state = m_metadata_transferred ? 1 : 0;

  mach_write_to_1(desc_loc + CLONE_LOC_META_OFFSET, sub_state);

  if (chunk_info != nullptr) {
    ut_ad(len > CLONE_DESC_LOC_BASE_LEN);

    auto len_left = len - CLONE_DESC_LOC_BASE_LEN;

    chunk_info->serialize(desc_loc + CLONE_DESC_LOC_BASE_LEN, len_left);
  }
}



void Clone_Desc_Locator::deserialize(const byte *desc_loc, uint desc_len,
                                     Chunk_Info *chunk_info) {
  m_header.deserialize(desc_loc, CLONE_DESC_HEADER_LEN);

  ut_ad(m_header.m_type == CLONE_DESC_LOCATOR);

  if (m_header.m_length < CLONE_DESC_LOC_BASE_LEN ||
      m_header.m_length > desc_len) {
    ut_ad(false);
    return;
  }

  m_clone_id = mach_read_from_8(desc_loc + CLONE_LOC_CID_OFFSET);
  m_snapshot_id = mach_read_from_8(desc_loc + CLONE_LOC_SID_OFFSET);

  m_clone_index = mach_read_from_4(desc_loc + CLONE_LOC_IDX_OFFSET);

  m_state = static_cast<Snapshot_State>(
      mach_read_from_1(desc_loc + CLONE_LOC_STATE_OFFSET));

  auto sub_state = mach_read_from_1(desc_loc + CLONE_LOC_META_OFFSET);
  m_metadata_transferred = (sub_state == 0) ? false : true;

  ut_ad(m_header.m_length >= CLONE_DESC_LOC_BASE_LEN);

  auto len_left = m_header.m_length - CLONE_DESC_LOC_BASE_LEN;

  if (chunk_info != nullptr && len_left != 0) {
    chunk_info->deserialize(desc_loc + CLONE_DESC_LOC_BASE_LEN, len_left);
  }
}
