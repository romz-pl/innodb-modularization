#pragma once

#include <innodb/univ/univ.h>

/** Result of comparing a path. */
enum class Fil_state {
  /** The path matches what was found during the scan. */
  MATCHES,

  /** No MLOG_FILE_DELETE record and the file could not be found. */
  MISSING,

  /** A MLOG_FILE_DELETE was found, file was deleted. */
  DELETED,

  /** Space ID matches but the paths don't match. */
  MOVED,

  /** Tablespace and/or filename was renamed. The DDL log will handle
  this case. */
  RENAMED
};
