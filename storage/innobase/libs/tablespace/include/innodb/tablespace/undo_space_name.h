#pragma once

#include <innodb/univ/univ.h>

/** This tablespace name is used as the prefix for implicit undo tablespaces
and during file discovery to open an undo tablespace before the DD is
recovered and available. */
static constexpr char undo_space_name[] = "innodb_undo";
