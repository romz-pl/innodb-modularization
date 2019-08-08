#pragma once

#include <innodb/univ/univ.h>

/** Writes encryption information to log header.
@param[in,out]  buf       log file header
@param[in]      key       encryption key
@param[in]      iv        encryption iv
@param[in]      is_boot   if it's for bootstrap */
bool log_file_header_fill_encryption(byte *buf, byte *key, byte *iv,
                                            bool is_boot);
