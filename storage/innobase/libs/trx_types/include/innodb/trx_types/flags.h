#pragma once

#include <innodb/univ/univ.h>


#define FSP_TRX_SYS_PAGE_NO \
  5 /*!< transaction        \
    system header, in       \
    tablespace 0 */

/** maximum length that a formatted trx_t::id could take, not including
the terminating NUL character. */
static const ulint TRX_ID_MAX_LEN = 17;

/** Page number of the transaction system page */
#define TRX_SYS_PAGE_NO FSP_TRX_SYS_PAGE_NO

/** Random value to check for corruption of trx_t */
static const ulint TRX_MAGIC_N = 91118598;

/** If this flag is set then the transaction cannot be rolled back
asynchronously. */
static const ib_uint32_t TRX_FORCE_ROLLBACK_DISABLE = 1 << 29;

/** Was the transaction rolled back asynchronously or by the
owning thread. This flag is relevant only if TRX_FORCE_ROLLBACK
is set.  */
static const ib_uint32_t TRX_FORCE_ROLLBACK_ASYNC = 1 << 30;

/** Mark the transaction for forced rollback */
static const ib_uint32_t TRX_FORCE_ROLLBACK = 1 << 31;

/** For masking out the above four flags */
static const ib_uint32_t TRX_FORCE_ROLLBACK_MASK = 0x1FFFFFFF;


/** Maximum transaction identifier */
#define TRX_ID_MAX IB_ID_MAX
