#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>

#ifndef UNIV_HOTBACKUP
#if defined(UNIV_HOTBACKUP) && defined(UNIV_DEBUG)

void meb_log_print_file_hdr(byte *block);

#endif /* UNIV_HOTBACKUP && UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */
