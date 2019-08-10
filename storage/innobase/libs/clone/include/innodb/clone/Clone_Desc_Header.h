#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Desc_Type.h>

/** Header common to all descriptors. */
struct Clone_Desc_Header {
  /** Descriptor version */
  uint m_version;

  /** Serialized length of descriptor in bytes */
  uint m_length;

  /** Descriptor type */
  Clone_Desc_Type m_type;

  /** Serialize the descriptor header: Caller must allocate
  the serialized buffer.
  @param[out]	desc_hdr	serialized header */
  void serialize(byte *desc_hdr);

  /** Deserialize the descriptor header.
  @param[in]	desc_hdr	serialized header
  @param[in]	desc_len	descriptor length
  @return true, if successful. */
  bool deserialize(const byte *desc_hdr, uint desc_len);
};
