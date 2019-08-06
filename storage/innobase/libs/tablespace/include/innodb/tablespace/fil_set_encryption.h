#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

#include <innodb/io/Encryption.h>

dberr_t fil_set_encryption(space_id_t space_id, Encryption::Type algorithm,
                           byte *key, byte *iv)
    MY_ATTRIBUTE((warn_unused_result));
