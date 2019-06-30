#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY
#ifdef UNIV_DEBUG

#define sync_check_iterate(A) true

#endif /* UNIV_DEBUG */
#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */
