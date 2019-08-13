#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

/** Dummy variable to catch access to uninitialized fields.  In the
debug version, dtuple_create() will make all fields of dtuple_t point
to data_error. */
extern byte data_error;

#endif /* UNIV_DEBUG */
