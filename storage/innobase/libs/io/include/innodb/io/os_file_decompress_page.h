#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

dberr_t os_file_decompress_page(bool dblwr_recover, byte *src, byte *dst,
                                ulint dst_len);
