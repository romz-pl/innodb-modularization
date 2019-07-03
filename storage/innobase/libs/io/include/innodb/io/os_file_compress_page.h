#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Compression.h>

byte *os_file_compress_page(Compression compression, ulint block_size,
                                   byte *src, ulint src_len, byte *dst,
                                   ulint *dst_len);
