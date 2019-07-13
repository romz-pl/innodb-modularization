#pragma once

#include <innodb/univ/univ.h>

/** @return true if the re-encrypt success */
bool fil_encryption_rotate() MY_ATTRIBUTE((warn_unused_result));
