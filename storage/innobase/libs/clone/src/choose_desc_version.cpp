#include <innodb/clone/choose_desc_version.h>

#include <innodb/clone/Clone_Desc_Header.h>
#include <innodb/clone/flags.h>


uint choose_desc_version(const byte *ref_loc) {
  if (ref_loc == nullptr) {
    return (CLONE_DESC_MAX_VERSION);
  }

  Clone_Desc_Header header;
  uint version;

  header.deserialize(ref_loc, CLONE_DESC_HEADER_LEN);
  version = header.m_version;

  /* Choose the minimum of remote locator version local
  supported version. */
  if (version > CLONE_DESC_MAX_VERSION) {
    version = CLONE_DESC_MAX_VERSION;
  }

  return (version);
}
