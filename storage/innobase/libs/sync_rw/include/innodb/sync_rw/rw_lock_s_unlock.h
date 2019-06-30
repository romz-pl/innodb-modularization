#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_s_unlock_gen.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define rw_lock_s_unlock(L) ((void)0)

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

#define rw_lock_s_unlock(L) rw_lock_s_unlock_gen(L, 0)

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
