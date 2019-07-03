#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_HOTBACKUP

/** De-allocates block cache at InnoDB shutdown. */
void meb_free_block_cache();

#endif /* UNIV_HOTBACKUP */
