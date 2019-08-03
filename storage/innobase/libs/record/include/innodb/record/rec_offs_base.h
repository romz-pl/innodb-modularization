#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/flag.h>

/* Get the base address of offsets.  The extra_size is stored at
this position, and following positions hold the end offsets of
the fields. */
#define rec_offs_base(offsets) (offsets + REC_OFFS_HEADER_SIZE)
