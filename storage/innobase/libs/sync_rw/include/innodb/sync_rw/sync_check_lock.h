#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define sync_check_lock(A, B) ((void)0)

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */
