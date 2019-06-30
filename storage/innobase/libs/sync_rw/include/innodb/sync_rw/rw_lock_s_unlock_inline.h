#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define rw_lock_s_unlock_inline(M, P, F, L) ((void)0)

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */
