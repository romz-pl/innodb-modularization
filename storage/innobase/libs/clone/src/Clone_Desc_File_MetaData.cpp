#include <innodb/clone/Clone_Desc_File_MetaData.h>

#include <innodb/clone/flags.h>
#include <innodb/machine/data.h>
#include <innodb/memory/mem_heap_alloc.h>

/** File Metadata: Snapshot state in 4 bytes */
static const uint CLONE_FILE_STATE_OFFSET = CLONE_DESC_HEADER_LEN;

/** File Metadata: File size in 8 bytes */
static const uint CLONE_FILE_SIZE_OFFSET = CLONE_FILE_STATE_OFFSET + 4;

/** File Metadata: Tablespace ID in 4 bytes */
static const uint CLONE_FILE_SPACE_ID_OFFSET = CLONE_FILE_SIZE_OFFSET + 8;

/** File Metadata: File index in 4 bytes */
static const uint CLONE_FILE_IDX_OFFSET = CLONE_FILE_SPACE_ID_OFFSET + 4;

/** File Metadata: First chunk number in 4 bytes */
static const uint CLONE_FILE_BCHUNK_OFFSET = CLONE_FILE_IDX_OFFSET + 4;

/** File Metadata: Last chunk number in 4 bytes */
static const uint CLONE_FILE_ECHUNK_OFFSET = CLONE_FILE_BCHUNK_OFFSET + 4;

/** File Metadata: File name length in 4 bytes */
static const uint CLONE_FILE_FNAMEL_OFFSET = CLONE_FILE_ECHUNK_OFFSET + 4;

/** File Metadata: File name */
static const uint CLONE_FILE_FNAME_OFFSET = CLONE_FILE_FNAMEL_OFFSET + 4;

/** File Metadata: Length excluding the file name */
static const uint CLONE_FILE_BASE_LEN = CLONE_FILE_FNAME_OFFSET;



void Clone_Desc_File_MetaData::init_header(uint version) {
  m_header.m_version = version;

  m_header.m_length = CLONE_FILE_BASE_LEN;
  m_header.m_length += static_cast<uint>(m_file_meta.m_file_name_len);

  m_header.m_type = CLONE_DESC_FILE_METADATA;
}

void Clone_Desc_File_MetaData::serialize(byte *&desc_file, uint &len,
                                         mem_heap_t *heap) {
  /* Allocate descriptor if needed. */
  if (desc_file == nullptr) {
    len = m_header.m_length;
    ut_ad(len == CLONE_FILE_FNAME_OFFSET + m_file_meta.m_file_name_len);

    desc_file = static_cast<byte *>(mem_heap_alloc(heap, len));
  } else {
    ut_ad(len >= m_header.m_length);
    len = m_header.m_length;
  }

  m_header.serialize(desc_file);

  mach_write_to_4(desc_file + CLONE_FILE_STATE_OFFSET, m_state);

  mach_write_to_8(desc_file + CLONE_FILE_SIZE_OFFSET, m_file_meta.m_file_size);
  mach_write_to_4(desc_file + CLONE_FILE_SPACE_ID_OFFSET,
                  m_file_meta.m_space_id);
  mach_write_to_4(desc_file + CLONE_FILE_IDX_OFFSET, m_file_meta.m_file_index);

  mach_write_to_4(desc_file + CLONE_FILE_BCHUNK_OFFSET,
                  m_file_meta.m_begin_chunk);
  mach_write_to_4(desc_file + CLONE_FILE_ECHUNK_OFFSET,
                  m_file_meta.m_end_chunk);

  mach_write_to_4(desc_file + CLONE_FILE_FNAMEL_OFFSET,
                  m_file_meta.m_file_name_len);

  /* Copy variable length file name. */
  if (m_file_meta.m_file_name_len != 0) {
    memcpy(static_cast<void *>(desc_file + CLONE_FILE_FNAME_OFFSET),
           static_cast<const void *>(m_file_meta.m_file_name),
           m_file_meta.m_file_name_len);
  }
}

bool Clone_Desc_File_MetaData::deserialize(const byte *desc_file,
                                           uint desc_len) {
  /* Deserialize the header and validate type and length. */
  if (desc_len < CLONE_FILE_BASE_LEN ||
      !m_header.deserialize(desc_file, desc_len) ||
      m_header.m_type != CLONE_DESC_FILE_METADATA) {
    return (false);
  }
  desc_len -= CLONE_FILE_BASE_LEN;

  uint int_type;
  int_type = mach_read_from_4(desc_file + CLONE_FILE_STATE_OFFSET);

  m_state = static_cast<Snapshot_State>(int_type);

  m_file_meta.m_file_size =
      mach_read_from_8(desc_file + CLONE_FILE_SIZE_OFFSET);
  m_file_meta.m_space_id =
      mach_read_from_4(desc_file + CLONE_FILE_SPACE_ID_OFFSET);
  m_file_meta.m_file_index =
      mach_read_from_4(desc_file + CLONE_FILE_IDX_OFFSET);

  m_file_meta.m_begin_chunk =
      mach_read_from_4(desc_file + CLONE_FILE_BCHUNK_OFFSET);
  m_file_meta.m_end_chunk =
      mach_read_from_4(desc_file + CLONE_FILE_ECHUNK_OFFSET);

  m_file_meta.m_file_name_len =
      mach_read_from_4(desc_file + CLONE_FILE_FNAMEL_OFFSET);

  ut_ad(m_header.m_length ==
        CLONE_FILE_FNAME_OFFSET + m_file_meta.m_file_name_len);

  /* Check if we have enough length. */
  if (desc_len < m_file_meta.m_file_name_len) {
    return (false);
  }

  if (m_file_meta.m_file_name_len == 0) {
    m_file_meta.m_file_name = nullptr;
  } else {
    m_file_meta.m_file_name =
        reinterpret_cast<const char *>(desc_file + CLONE_FILE_FNAME_OFFSET);
    auto last_char = m_file_meta.m_file_name[m_file_meta.m_file_name_len - 1];

    /* File name must be NULL terminated. */
    if (last_char != '\0') {
      return (false);
    }
  }
  return (true);
}
