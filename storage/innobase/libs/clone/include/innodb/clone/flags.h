#pragma once

#include <innodb/univ/univ.h>

/** Invalid locator ID. */
const ib_uint64_t CLONE_LOC_INVALID_ID = 0;

/** Maximum base length for any serialized descriptor. This is only used for
optimal allocation and has no impact on version compatibility. */
const uint32_t CLONE_DESC_MAX_BASE_LEN = 64;

/** Align by 4K for O_DIRECT */
const uint32_t CLONE_ALIGN_DIRECT_IO = 4 * 1024;

/** Maximum number of concurrent tasks for each clone */
const int CLONE_MAX_TASKS = 64;


/** Maximum supported descriptor version. The version represents the current
set of descriptors and its elements. */
const uint CLONE_DESC_MAX_VERSION = 100;

/** Header: Version is in first 4 bytes */
const uint CLONE_DESC_VER_OFFSET = 0;

/** Header: Total length is stored in next 4 bytes */
const uint CLONE_DESC_LEN_OFFSET = CLONE_DESC_VER_OFFSET + 4;

/** Header: Descriptor type is in next 4 bytes */
const uint CLONE_DESC_TYPE_OFFSET = CLONE_DESC_LEN_OFFSET + 4;

/** Header: Fixed length. */
const uint CLONE_DESC_HEADER_LEN = CLONE_DESC_TYPE_OFFSET + 4;


/** Locator: Clone identifier in 8 bytes */
const uint CLONE_LOC_CID_OFFSET = CLONE_DESC_HEADER_LEN;

/** Locator: Snapshot identifier in 8 bytes */
const uint CLONE_LOC_SID_OFFSET = CLONE_LOC_CID_OFFSET + 8;

/** Locator: Clone array index in 4 bytes */
const uint CLONE_LOC_IDX_OFFSET = CLONE_LOC_SID_OFFSET + 8;

/** Locator: Clone Snapshot state in 1 byte */
const uint CLONE_LOC_STATE_OFFSET = CLONE_LOC_IDX_OFFSET + 4;

/** Locator: Clone Snapshot sub-state in 1 byte */
const uint CLONE_LOC_META_OFFSET = CLONE_LOC_STATE_OFFSET + 1;

/** Locator: Total length */
const uint CLONE_DESC_LOC_BASE_LEN = CLONE_LOC_META_OFFSET + 1;












