#pragma once

#include <innodb/univ/univ.h>

extern "C" {

void page_zip_free(
    void *opaque MY_ATTRIBUTE((unused)),  /*!< in: memory heap */
    void *address MY_ATTRIBUTE((unused))); /*!< in: object to free */

}
