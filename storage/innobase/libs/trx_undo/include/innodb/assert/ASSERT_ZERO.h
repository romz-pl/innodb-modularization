#pragma once

#include <innodb/univ/univ.h>

#include <innodb/assert/assert.h>

#include <string.h>
#include <utility>

/** Assert that a block of memory is filled with zero bytes.
Compare at most sizeof(field_ref_zero) bytes.
@param b in: memory block
@param s in: size of the memory block, in bytes */
#define ASSERT_ZERO(b, s)          \
  ut_ad(!memcmp(b, field_ref_zero, \
                std::min(static_cast<size_t>(s), sizeof field_ref_zero)));
