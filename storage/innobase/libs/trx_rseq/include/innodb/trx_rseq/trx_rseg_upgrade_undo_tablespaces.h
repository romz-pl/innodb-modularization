#pragma once

#include <innodb/univ/univ.h>

/** Upgrade the TRX_SYS page so that it no longer tracks rsegs in undo
tablespaces other than the system tablespace.  Add these tablespaces to
undo::spaces and put FIL_NULL in the slots in TRX_SYS.*/
void trx_rseg_upgrade_undo_tablespaces();
