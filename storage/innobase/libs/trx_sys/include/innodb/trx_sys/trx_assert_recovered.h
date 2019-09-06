#pragma once

#include <innodb/univ/univ.h>

#if defined UNIV_DEBUG || defined UNIV_BLOB_LIGHT_DEBUG
/** Assert that a transaction has been recovered.
 @return true */
UNIV_INLINE
ibool trx_assert_recovered(trx_id_t trx_id) /*!< in: transaction identifier */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_DEBUG || UNIV_BLOB_LIGHT_DEBUG */
