#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Block.h>
#include <innodb/io/IORequest.h>

Block *os_file_encrypt_log(const IORequest &type, void *&buf,
                                  byte *&scratch, ulint *n);
