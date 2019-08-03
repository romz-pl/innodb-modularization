#pragma once

#include <innodb/univ/univ.h>

/* The infimum and supremum records are omitted from the compressed page.
On compress, we compare that the records are there, and on uncompress we
restore the records. */
/** Extra bytes of an infimum record */
static const byte infimum_extra[] = {
    0x01,          /* info_bits=0, n_owned=1 */
    0x00, 0x02     /* heap_no=0, status=2 */
    /* ?, ?	*/ /* next=(first user rec, or supremum) */
};
