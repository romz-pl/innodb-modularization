#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_get_mutex.h>
#include <innodb/hash/hash_table_t.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Assert that the mutex for the table is held */
#define HASH_ASSERT_OWN(TABLE, FOLD)              \
  ut_ad((TABLE)->type != HASH_TABLE_SYNC_MUTEX || \
        (mutex_own(hash_get_mutex((TABLE), FOLD))));

#else /* !UNIV_HOTBACKUP */

#define HASH_ASSERT_OWN(TABLE, FOLD)

#endif /* !UNIV_HOTBACKUP */
