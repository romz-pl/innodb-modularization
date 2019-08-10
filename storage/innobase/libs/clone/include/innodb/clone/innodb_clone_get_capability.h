#pragma once

#include <innodb/univ/univ.h>

#include "sql/handler.h"

/** Get capability flags for clone operation
@param[out]	flags	capability flag */
void innodb_clone_get_capability(Ha_clone_flagset &flags);
