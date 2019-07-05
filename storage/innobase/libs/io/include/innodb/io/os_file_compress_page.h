#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Compression.h>
#include <innodb/io/Block.h>
#include <innodb/io/IORequest.h>

byte *os_file_compress_page(Compression compression, ulint block_size,
                                   byte *src, ulint src_len, byte *dst,
                                   ulint *dst_len);


Block *os_file_compress_page(IORequest &type, void *&buf, ulint *n);
