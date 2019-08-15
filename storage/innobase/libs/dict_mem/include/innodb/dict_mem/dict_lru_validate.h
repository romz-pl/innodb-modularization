#pragma once

#include <innodb/univ/univ.h>


#ifdef UNIV_HOTBACKUP

#define dict_lru_validate(x) (true)

#else

#ifdef UNIV_DEBUG

/** Validate the dictionary table LRU list.
 @return true if validate OK */
static ibool dict_lru_validate(void);

#endif /* UNIV_DEBUG */
#endif /* UNIV_HOTBACKUP */
