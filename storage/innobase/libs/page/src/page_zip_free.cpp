#include <innodb/page/page_zip_free.h>

extern "C" {

/** Deallocate memory for zlib. */
void page_zip_free(
    void *opaque MY_ATTRIBUTE((unused)),  /*!< in: memory heap */
    void *address MY_ATTRIBUTE((unused))) /*!< in: object to free */
{}

} /* extern "C" */
