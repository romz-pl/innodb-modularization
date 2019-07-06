#include <innodb/tablespace/Tablespace_files.h>

#include "dict0dict.h"

/** Get the file names that map to a space ID
@param[in]	space_id	Tablespace ID
@return the filenames that map to space id */
Tablespace_files::Names *Tablespace_files::find(space_id_t space_id) {
  ut_ad(space_id != TRX_SYS_SPACE);

  if (dict_sys_t::is_reserved(space_id) &&
      space_id != dict_sys_t::s_space_id) {
    auto it = m_undo_paths.find(space_id);

    if (it != m_undo_paths.end()) {
      return (&it->second);
    }

  } else {
    auto it = m_ibd_paths.find(space_id);

    if (it != m_ibd_paths.end()) {
      return (&it->second);
    }
  }

  return (nullptr);
}


bool Tablespace_files::erase(space_id_t space_id) {
  ut_ad(space_id != TRX_SYS_SPACE);

  if (dict_sys_t::is_reserved(space_id) &&
      space_id != dict_sys_t::s_space_id) {
    auto n_erased = m_undo_paths.erase(space_id);

    return (n_erased == 1);
  } else {
    auto n_erased = m_ibd_paths.erase(space_id);

    return (n_erased == 1);
  }

  return (false);
}
