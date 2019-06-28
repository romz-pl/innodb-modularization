#include <innodb/allocator/ut_new_boot.h>

#include "mysql/psi/mysql_memory.h"

#include <innodb/allocator/psi.h>

/** Setup the internal objects needed for UT_NEW() to operate.
This must be called before the first call to UT_NEW(). */
void ut_new_boot() {
#ifdef UNIV_PFS_MEMORY
  for (size_t i = 0; i < n_auto; i++) {
    /* e.g. "btr0btr" */
    pfs_info_auto[i].m_name = auto_event_names[i];

    /* a pointer to the pfs key */
    pfs_info_auto[i].m_key = &auto_event_keys[i];

    pfs_info_auto[i].m_flags = 0;
    pfs_info_auto[i].m_volatility = PSI_VOLATILITY_UNKNOWN;
    pfs_info_auto[i].m_documentation = PSI_DOCUMENT_ME;
  }

  // This causes compilation error:
  // innodb/univ/univ.h:486: error: invalid application of ‘sizeof’ to incomplete type ‘PSI_memory_info [] {aka PSI_memory_info_v1 []}’
  //
  // PSI_MEMORY_CALL(register_memory)("innodb", pfs_info, UT_ARR_SIZE(pfs_info));
  //
  // Hence it was replaced by constant: 16, see file psi.cpp, where the definition of pfs_info is located
  PSI_MEMORY_CALL(register_memory)("innodb", pfs_info, 16);

  PSI_MEMORY_CALL(register_memory)("innodb", pfs_info_auto, n_auto);
#endif /* UNIV_PFS_MEMORY */
}
