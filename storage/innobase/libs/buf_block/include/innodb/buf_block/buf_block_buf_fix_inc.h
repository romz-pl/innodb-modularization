#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_block/buf_block_buf_fix_inc_func.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Increments the bufferfix count.
@param[in,out]	b	block to bufferfix
@param[in]	f	file name where requested
@param[in]	l	line number where requested */
#define buf_block_buf_fix_inc(b, f, l) buf_block_buf_fix_inc_func(f, l, b)

#else /* UNIV_DEBUG */

/** Increments the bufferfix count.
@param[in,out]	b	block to bufferfix
@param[in]	f	file name where requested
@param[in]	l	line number where requested */
#define buf_block_buf_fix_inc(b, f, l) buf_block_buf_fix_inc_func(b)

#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */


