#pragma once

#include <innodb/univ/univ.h>

/** This tablespace name is used internally during file discovery to open a
general tablespace before the data dictionary is recovered and available. */
static constexpr char general_space_name[] = "innodb_general";
