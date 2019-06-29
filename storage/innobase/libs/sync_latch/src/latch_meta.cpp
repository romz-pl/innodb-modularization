#include <innodb/sync_latch/latch_meta.h>

#ifndef UNIV_LIBRARY

/* Meta data for all the InnoDB latches. If the latch is not in recorded
here then it will be be considered for deadlock checks.  */
LatchMetaData latch_meta;

#endif
