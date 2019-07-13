#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG
#ifndef UNIV_HOTBACKUP

bool os_aio_validate_skip();

#endif /* !UNIV_HOTBACKUP */
#endif /* UNIV_DEBUG */
