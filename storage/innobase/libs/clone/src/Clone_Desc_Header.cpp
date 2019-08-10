#include <innodb/clone/Clone_Desc_Header.h>

#include <innodb/assert/assert.h>
#include <innodb/clone/flags.h>
#include <innodb/machine/data.h>


void Clone_Desc_Header::serialize(byte *desc_hdr) {
  mach_write_to_4(desc_hdr + CLONE_DESC_VER_OFFSET, m_version);
  mach_write_to_4(desc_hdr + CLONE_DESC_LEN_OFFSET, m_length);
  mach_write_to_4(desc_hdr + CLONE_DESC_TYPE_OFFSET, m_type);
}

bool Clone_Desc_Header::deserialize(const byte *desc_hdr, uint desc_len) {
  if (desc_len < CLONE_DESC_HEADER_LEN) {
    return (false);
  }
  m_version = mach_read_from_4(desc_hdr + CLONE_DESC_VER_OFFSET);
  m_length = mach_read_from_4(desc_hdr + CLONE_DESC_LEN_OFFSET);

  uint int_type;
  int_type = mach_read_from_4(desc_hdr + CLONE_DESC_TYPE_OFFSET);
  ut_ad(int_type < CLONE_DESC_MAX);

  m_type = static_cast<Clone_Desc_Type>(int_type);
  return (true);
}
