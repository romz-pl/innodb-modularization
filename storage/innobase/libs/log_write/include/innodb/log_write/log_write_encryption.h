#pragma once

#include <innodb/univ/univ.h>

/** Write the encryption info into the log file header(the 3rd block).
It just need to flush the file header block with current master key.
@param[in]	key	encryption key
@param[in]	iv	encryption iv
@param[in]	is_boot	if it is for bootstrap
@return true if success. */
bool log_write_encryption(byte *key, byte *iv, bool is_boot);
