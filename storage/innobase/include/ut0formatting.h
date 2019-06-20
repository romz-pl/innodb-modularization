#pragma once

///
/// Define formatting strings
///


#ifdef _WIN32 /* Use the formatting strings defined in Visual Studio. */
#   define UINT32PF "%lu"
#   define UINT32PFS "lu"
#   define UINT64PF "%llu"
#   define UINT64PFx "%016llx"
#else /* Use the formatting strings defined in the C99 standard. */
#   include <inttypes.h>
#   define UINT32PF "%" PRIu32
#   define UINT32PFS PRIu32
#   define UINT64PF "%" PRIu64
#   define UINT64PFx "%016" PRIx64
#endif /* _WIN32 */

#define IB_ID_FMT UINT64PF


#ifdef _WIN64
#   define ULINTPFS "llu"
#else
#   define ULINTPFS "lu"
#endif /* _WIN64 */

#define ULINTPF "%" ULINTPFS
