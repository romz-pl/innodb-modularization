#pragma once

#include <innodb/univ/univ.h>

#include <innodb/assert/assert.h>

#include <string.h>
#include <utility>

/** Assert that a BLOB pointer is filled with zero bytes.
@param b in: BLOB pointer */
#define ASSERT_ZERO_BLOB(b) \
  ut_ad(!memcmp(b, field_ref_zero, sizeof field_ref_zero))
