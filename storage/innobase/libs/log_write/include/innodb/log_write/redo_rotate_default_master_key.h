#pragma once

#include <innodb/univ/univ.h>

/** Rotate default master key for redo log encryption. */
void redo_rotate_default_master_key();
