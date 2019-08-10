#include <innodb/clone/clone_validate_locator.h>

#include <innodb/assert/assert.h>
#include <innodb/clone/Clone_Desc_Header.h>
#include <innodb/clone/flags.h>

bool clone_validate_locator(const byte *desc_loc, uint desc_len) {
  Clone_Desc_Header header;

  if (!header.deserialize(desc_loc, desc_len)) {
    ut_ad(false);
    return (false);
  }
  if (desc_len < CLONE_DESC_LOC_BASE_LEN ||
      header.m_length < CLONE_DESC_LOC_BASE_LEN || header.m_length > desc_len ||
      header.m_type != CLONE_DESC_LOCATOR) {
    ut_ad(false);
    return (false);
  }
  return (true);
}
